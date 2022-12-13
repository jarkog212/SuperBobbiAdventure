#pragma once
#include "State.h"
#include <graphics/font.h>
#include "GameObject_2D.h"
#include <graphics/sprite_renderer.h>
#include <audio/audio_manager.h>
#include <functional>
#include <map>
#include "Globals.h"

#define BUTTON_DELAY 0.2f                                    //button cooldown of 0.2 seconds, fix for the gef input controller problem

//external link to the global variables
extern Globals globals;

//templated struct, allows for various things to happen with menus
template <typename T>
struct MenuWidget {
    gef::Vector4 position_;                                  //general position of the menu on screen
    gef::Sprite selector_;                                   //potential sprite for little icon next to the selcted item, not used
    gef::Texture* selector_T_ = NULL;                        //texture for selector
    gef::AudioManager* audio_ = NULL;                        //pointer to global audio, used for sfx
    int current_index_ = 0;                                  //index of the currently selected item
    int highlight_colour_ = (int)0xFFFF00FF;                 //colour of the selected text
    int base_colour_ = (int)0xFFFFFFFF;                      //colour of the base text
    std::vector<Text> elements_;                             //collection of the texts in the menu, indexed hence vector is used
    std::vector<std::function<void(T)>> element_activity_;   //collection of functions pretaining to the menu items, indeces match with menu elements

    MenuWidget() {};                                 
    MenuWidget(gef::Platform& platform, const char* address_selector, SpriteSetup setup) {
        GameObject_2D* temp = new GameObject_2D(platform, setup, NULL);                                //dummy for texture loading
        selector_T_ = temp->getTexture();                                                              //gets the texture
        temp->setProtectTexture(true);                                                                 //disbles the deletion of the texture with the holder
        delete temp;                                                                                   //delete the dummy
        //setup selector
        selector_.set_texture(selector_T_);
        selector_.set_position(setup.posX, setup.posY, setup.ordZ);
        selector_.set_width(setup.width);
        selector_.set_height(setup.height);
    }

    ~MenuWidget() {
        if (selector_T_) {
            delete selector_T_;
            selector_T_ = NULL;
        }
    }

    void ChangeBaseColour(int newColour) 
    {
        base_colour_ = newColour;
        for (auto it : elements_)
            it.colour_ = newColour;
    }

    void RenderMenu(gef::SpriteRenderer* sprite_renderer, gef::Font* font) {
        for (int i = 0; i < elements_.size(); ++i) {                                 //iterate through elements, reset their colour, set their position (relative to the menu) and render
            if (i == current_index_) {                                               //set the currect element's colour to the selected colour
                elements_[i].colour_ = highlight_colour_;
                selector_.set_position(elements_[i].position_ - Vector4(0.5, 0, 0));
            }
            else 
                elements_[i].colour_ = base_colour_;                                 //everything else is set to base

            elements_[i].position_ += position_;
            elements_[i].displayText(sprite_renderer, font);                         //font is dependent on the parameter
            elements_[i].position_ -= position_;

        }
        if (selector_T_) {                                                           //draw selector if texture is present
            sprite_renderer->DrawSprite(selector_);
        }
    }

    void Select(T stuff) {                                                           //call the function form the element_activity array based on the current index
        if (globals.G_button_down_timer <= 0) {                                      //ensure the button cooldown is depleted
            if (audio_) {                                                            //play sound if manager is available
                audio_->PlaySample(globals.SFX_map["Menu_select"], false);
            }
            globals.G_button_down_timer = BUTTON_DELAY;                              //add to the button cooldown
            auto it = element_activity_.at(current_index_); 
            it(stuff);                                                               //call the function
        }
    }

    void Next() {                                                                    //switch to next current index, select next menu element if available 
        if (globals.G_button_down_timer <= 0) {
            if (audio_) {                                                            //play sound if manager is available
                audio_->PlaySample(globals.SFX_map["Menu_move"], false);
            }
            globals.G_button_down_timer = BUTTON_DELAY;                              //add to button cooldown
            ++current_index_;
            current_index_ %= elements_.size();                                      //safegoard against "out of bounds"
        }
    }

    void Previous() {                                                                //same/similar to next()
        if (globals.G_button_down_timer <= 0) {
            if (audio_) {
                audio_->PlaySample(globals.SFX_map["Menu_move"], false);
            }
            globals.G_button_down_timer = BUTTON_DELAY;
            --current_index_;
            if (current_index_ < 0) {
                current_index_ = elements_.size() - 1;
            }
        }
    }

    void AddElement(Text text, std::function<void(T)> activity) {                    //adds a menu element, require the text and labda fr the action, this means meny can have versatile effects as well as texts
        elements_.push_back(text);
        element_activity_.push_back(activity);
    }

    void RemoveElement(std::string in) {
        auto it = std::find_if(elements_.begin(), elements_.end(), [&in](Text i) {return i.text_ == in;});
        if (it != elements_.end())
        {
            int ID = it - elements_.begin();
            elements_.erase(it);
            element_activity_.erase(element_activity_.begin() + ID);
        }
    }
}; 