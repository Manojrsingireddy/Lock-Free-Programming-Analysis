#include "BST.h"
#define BAD_VAL -1
using namespace std;

lock_based_bst::lock_based_bst(){
    root = NULL;
    pthread_mutex_init(&lock, NULL);
}

lock_based_bst::~lock_based_bst(){
    pthread_mutex_destroy(&lock);
}

int lock_based_bst::get(int key){
    pthread_mutex_lock(&lock);
    bst_node * curr = root;
    int val = BAD_VAL;
    while(curr != NULL){
        if(curr->key == key){
            val = curr->val;
            break;
        }
        else if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    pthread_mutex_unlock(&lock);
    return val;
}

void lock_based_bst::insert(int key, int val){
    pthread_mutex_lock(&lock);
    if(root == nullptr){
        root = new bst_node(key, val);
        pthread_mutex_unlock(&lock);
        return;
    }
    bst_node * curr = root;
    bst_node * prev = nullptr;
    while(curr){
        prev = curr;
        if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    if(key < prev->key){
        prev->left = new bst_node(key, val);
    }
    else{
        prev->right = new bst_node(key, val);
    }
    pthread_mutex_unlock(&lock);    
}

void lock_based_bst::remove(int key){
    pthread_mutex_lock(&lock);
    bst_node * curr = root;
    bst_node * prev = nullptr;
    while(curr){
        if(curr->key == key){
            break;
        }
        prev = curr;
        if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    if(curr == nullptr){
        pthread_mutex_unlock(&lock);
        return;
    }
    if(curr->left == nullptr && curr->right == nullptr){
        if(prev == nullptr){
            root = nullptr;
        }
        else if(prev->left == curr){
            prev->left = nullptr;
        }
        else{
            prev->right = nullptr;
        }
        delete curr;
    }
    else if(curr->left == nullptr){
        if(prev == nullptr){
            root = curr->right;
        }
        else if(prev->left == curr){
            prev->left = curr->right;
        }
        else{
            prev->right = curr->right;
        }
        delete curr;
    }
    else if(curr->right == nullptr){
        if(prev == nullptr){
            root = curr->left;
        }
        else if(prev->left == curr){
            prev->left = curr->left;
        }
        else{
            prev->right = curr->left;
        }
        delete curr;
    }
    else{
        bst_node * succ = curr->right;
        bst_node * succ_prev = curr;
        while(succ->left){
            succ_prev = succ;
            succ = succ->left;
        }
        curr->key = succ->key;
        curr->val = succ->val;
        if(succ_prev->left == succ){
            succ_prev->left = succ->right;
        }
        else{
            succ_prev->right = succ->right;
        }
        delete succ;
    }
    pthread_mutex_unlock(&lock);
}

lock_free_bst::lock_free_bst() : root(nullptr) {}

void delete_subtree(lf_bst_node *node) {
    if (node) {
        delete_subtree(node->left.load());
        delete_subtree(node->right.load());
        delete node;
    }
}

lock_free_bst::~lock_free_bst() {
    delete_subtree(root.load());
}


int lock_free_bst::get(int key){
    lf_bst_node * curr = root.load();
    while(curr){
        if(curr->key == key){
            return curr->val;
        }
        else if(curr->key > key){
            curr = curr->left.load();
        }
        else{
            curr = curr->right.load();
        }
    }
    return BAD_VAL;
}

bool try_remove(Info * info){
    lf_bst_node *old_node = info->old_node;
    lf_bst_node *new_node = info->new_node;

    lf_bst_node *left_child = old_node->left.load();
    lf_bst_node *right_child = old_node->right.load();

    if (left_child && right_child) {
        lf_bst_node *smallest_right = right_child;
        atomic<lf_bst_node *> *smallest_right_parent_ptr = &old_node->right;

        while (smallest_right->left.load()) {
            smallest_right_parent_ptr = &smallest_right->left;
            smallest_right = smallest_right->left.load();
        }

        new_node = new lf_bst_node(smallest_right->key, smallest_right->val);
        new_node->left.store(left_child);
        new_node->right.store(right_child);

        Info *delete_info = new Info(Info::Type::Delete, smallest_right, nullptr);
        if (smallest_right->op_info.compare_exchange_strong(nullptr, delete_info)) {
            help(delete_info);
            delete delete_info;
        } else {
            delete new_node;
            return false;
        }
    } else {
        new_node = left_child ? left_child : right_child;
    }

    atomic<lf_bst_node *> *child_ptr = (old_node->key < new_node->key) ? &old_node->right : &old_node->left;
    return child_ptr->compare_exchange_strong(old_node, new_node);
}

void help(Info *info) {
    if (info->type == Info::Type::Insert) {
        lf_bst_node *old_node = info->old_node;
        lf_bst_node *new_node = info->new_node;
        atomic<lf_bst_node *> *child_ptr = (old_node->key < new_node->key) ? &old_node->right : &old_node->left;
        child_ptr->compare_exchange_strong(old_node, new_node);
    } else if (info->type == Info::Type::Delete) {
        lf_bst_node *old_node = info->old_node;
        lf_bst_node *new_node = info->new_node;
        try_remove(info);
    }
}


void lock_free_bst::insert(int key, int val){
    lf_bst_node *new_node = new lf_bst_node(key, val);
    lf_bst_node *current = root.load();
    atomic<lf_bst_node *> *parent_ptr = nullptr;

    while (true) {
        if (!current) {
            if (!parent_ptr->compare_exchange_strong(current, new_node))
                continue;
            return;
        }

        Info *current_info = current->op_info.load();
        if (current_info) {
            help(current_info);
            continue;
        }

        if (key < current->key) {
            parent_ptr = &current->left;
            current = current->left.load();
        } else if (key > current->key) {
            parent_ptr = &current->right;
            current = current->right.load();
        } else {
            return; // Key already exists
        }

        Info *insert_info = new Info(Info::Type::Insert, current, new_node);
        if (current->op_info.compare_exchange_strong(nullptr, insert_info)) {
            help(insert_info);
            delete insert_info;
            return;
        } else {
            delete insert_info;
        }
    }
}

void lock_free_bst::remove(int key){
    lf_bst_node *current = root.load();
    atomic<lf_bst_node *> *parent_ptr = nullptr;

    while (current) {
        Info *current_info = current->op_info.load();
        if (current_info) {
            help(current_info);
            continue;
        }

        if (key < current->key) {
            parent_ptr = &current->left;
            current = current->left.load();
        } else if (key > current->key) {
            parent_ptr = &current->right;
            current = current->right.load();
        } else {
            lf_bst_node *replacement = nullptr;
            if (current->left.load() && current->right.load()) {
                remove(current->right.load()->key);
                replacement = current->right.load();
            } else {
                replacement = (current->left.load()) ? current->left.load() : current->right.load();
            }

            Info *delete_info = new Info(Info::Type::Delete, current, replacement);
            if (current->op_info.compare_exchange_strong(nullptr, delete_info)) {
                if (try_remove(delete_info)) {
                    help(delete_info);
                    delete delete_info;
                    return;
                } else {
                    delete delete_info;
                }
            }
        }
    }
    throw std::runtime_error("Key not found in the tree");
}


