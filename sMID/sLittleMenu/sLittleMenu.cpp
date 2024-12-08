#include "sLittleMenu.hpp"






sLittleMenu::sLittleMenu(){

}

sLittleMenu::~sLittleMenu(){
    delete menu;
    delete curr_menu;
}


void sLittleMenu::init(){
    sLM_MenuItemData root = {.id = 0,.text = "This is root",.show_para_type = sLM_ISPType::STRING,.para_str = "root str"};

    menu = sLM_Menu::create(&root,sizeof(sLM_MenuItemData));
    curr_menu = menu;
}

sLM_Menu* sLittleMenu::createMenu(const void* src_data, size_t dsize){
    return sLM_Menu::create(src_data,dsize);
}

int sLittleMenu::addSub(sLM_Menu* submenu){
    menu->addSubMenu(submenu);
    return 0;
}

void sLittleMenu::opEnter(){
    if(curr_menu->child != nullptr){
        curr_menu = curr_menu->child;
    }
}

void sLittleMenu::opBack(){
    if(curr_menu->parent != nullptr){
        curr_menu = curr_menu->parent;
    }
}

void sLittleMenu::opPrev(){
    
}

void sLittleMenu::opNext(){
    if(curr_menu->next_sibling != nullptr){
        curr_menu = curr_menu->next_sibling;
    }
}

void sLittleMenu::update(){

}


