//
//  main.cpp
//  merkletree
//
//  Created by Patrik Nusszer on 2020. 07. 12..
//  Copyright © 2020. Patrik Nusszer. All rights reserved.
//

#include <iostream>
#include <string>
#include "sha512.h"
#include <filesystem>
#include <fstream>
#include <math.h>

using namespace std;
using namespace std::filesystem;

struct node {
    struct node *left;
    struct node *right;
    string digest;
};

string hashOf(unsigned char *message, unsigned int msglen) {
    unsigned char digest[SHA512::DIGEST_SIZE];
    memset(digest,0,SHA512::DIGEST_SIZE);
    SHA512 ctx = SHA512();
    ctx.init();
    ctx.update(message, msglen);
    ctx.final(digest);
    char buf[2*SHA512::DIGEST_SIZE+1];
    buf[2*SHA512::DIGEST_SIZE] = 0;
    
    for (int i = 0; i < SHA512::DIGEST_SIZE; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    
    return std::string(buf);
}

string buildmerklev2(string* heap, ifstream &f, size_t sz, size_t i, size_t len) {
    if (i * 2 + 1 >= len) {
        int buffsz;
        char buff[(buffsz = sz / ((len + 1) / 2))];
        f.read(buff, buffsz);
        return hashOf((unsigned char*)buff, (unsigned int)f.gcount());
    }
    
    heap[i * 2 + 1] = buildmerklev2(heap, f, sz, i * 2 + 1, len);
    heap[i * 2 + 2] = buildmerklev2(heap, f, sz, i * 2 + 2, len);
    string msg = heap[i * 2 + 1] + heap[i * 2 + 2];
    heap[i] = hashOf((unsigned char*)msg.c_str(), (unsigned int)msg.length());
    return heap[i];
}

string *buildmerklev2(ifstream &f, size_t sz, int exponent, size_t &len) {
    string* heap = new string[len = (exponent * 2 - 1)];
    buildmerklev2(heap, f, sz, 0, len);
    return heap;
}

struct node *buildmerklev1(ifstream &f, size_t sz, int lvl, int depth) {
    if (depth == lvl) {
        int buffsz;
        char buff[(buffsz = sz / lvl)];
        f.read(buff, buffsz);
        struct node *n = new node();
        n->left = nullptr;
        n->right = nullptr;
        n->digest = hashOf((unsigned char*)buff, (unsigned int)f.gcount());
        return n;
    }
    
    struct node *left = buildmerklev1(f, sz, lvl + 1, depth);
    struct node *right = buildmerklev1(f, sz, lvl + 1, depth);
    
    struct node *n = new node();
    n->left = left;
    n->right = right;
    string msg = n->left->digest + n->right->digest;
    n->digest = hashOf((unsigned char*)msg.c_str(), (unsigned int)msg.length());
    return n;
}

int main(int argc, const char * argv[]) {
    ifstream input = ifstream("cegesautok.pdf");
    size_t len;
    string* merkleroot = buildmerklev2(input, file_size("cegesautok.pdf"), 16, len);
    cout << *merkleroot << endl;
    return 0;
}
