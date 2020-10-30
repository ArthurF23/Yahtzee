// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#pragma disable(warning: 4996)
#include "imgui.h"
using namespace ImGui;

#include <random>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdio>
using namespace std;

#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <d3d9.h>
#pragma comment (lib, "d3d9.lib") //Adds some thing that make d3d9.h work
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

static bool GAMEOVER = false;

static char path[124] = "PATH";
// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int player_num = 1;
int times_rolled = 0;

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

int random_generator(int x, int y) {    
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(x, y); // guaranteed unbiased
    int random_int = uni(rng);  
    //cout << x << " " << y << " " << random_int << endl;
    if (random_int < x || random_int > y) {
        //cout << "RETURN" << endl;
        return random_generator(x, y);
    }

    return random_int;
}

class DICE {
protected:
    int faces[6] = { 1, 2, 3, 4, 5, 6 };

public:

    void lock(bool TF) {
        if (times_rolled != 0) {
            is_locked = TF;
        }
        else {

        }
    };


    atomic<bool> is_locked = false;

    atomic<char> current_face = 0;

    void roll() {
        if (is_locked == false) {
            current_face = faces[random_generator(0, 5)];
        };
    };

    void reset() {
        current_face = 0;
        is_locked = false;
    }

};

class player {
public:   

    int turns_taken = 0;

    atomic<int> upper_score = 0;

    atomic<int> lower_score = 0;

    atomic<int> total_score = 0;

    char player_inputs_1[1024 * 16] = "0";

    char player_inputs_2[1024 * 16] = "0";

    char player_inputs_3[1024 * 16] = "0";

    char player_inputs_4[1024 * 16] = "0";

    char player_inputs_5[1024 * 16] = "0";

    char player_inputs_6[1024 * 16] = "0";

    char three_of_kind[1024 * 16] = "0";

    char four_of_kind[1024 * 16] = "0";

    bool fullhouse = false;

    bool sm_straight = false;

    bool lg_straight = false;

    bool scored_yahtzee = false;

    char chance[1024 * 16] = "0";

    int yahtzee_bonus = 0;

    void reset() {
        turns_taken = 0;
        upper_score = 0;
        lower_score = 0;
        total_score = 0;
        fullhouse = false;
        sm_straight = false;
        lg_straight = false;
        scored_yahtzee = false;
        yahtzee_bonus = 0;
        for (int i = 0; i < 1024 * 16; i++) {
            if (i == 0) {
                player_inputs_1[i] = (char)"0";
                player_inputs_2[i] = (char)"0";
                player_inputs_3[i] = (char)"0";
                player_inputs_4[i] = (char)"0";
                player_inputs_5[i] = (char)"0";
                player_inputs_6[i] = (char)"0";
                chance[i] = (char)"0";
            }
            else {
                player_inputs_1[i] = (char)"";
                player_inputs_2[i] = (char)"";
                player_inputs_3[i] = (char)"";
                player_inputs_4[i] = (char)"";
                player_inputs_5[i] = (char)"";
                player_inputs_6[i] = (char)"";
                chance[i] = (char)"";
            };
        };
    };
};

struct MAINWINDOW {
    int WIDTH = 1280;
    int HEIGHT = 800;
};

struct CARD_SIZE {
    int WIDTH = 400;
    int HEIGHT = 500;
};

struct HELP_WINDOW {
    int WIDTH = 450;
    int HEIGHT = 700;
};


DICE* Dice_1;
DICE* Dice_2;
DICE* Dice_3;
DICE* Dice_4;
DICE* Dice_5;

player* P_1;
player* P_2;
player* P_3;
player* P_4;

player* pointer;

int player_order[4] = {-1, -1, -1 ,-1};

void turn_cycle() {
    int last_num[4] = { 10, 10, 10, 10 };

    for (int i = 0; i < player_num; i += 0) {
        int num;
        //thread th(random_generator, (0, player_num - 1)) = num;
        //cout << player_num;
        num = random_generator(0, player_num - 1);
        //cout << " " << num << endl;
        //th.join();

        if (num != last_num[0] && num != last_num[1] && num != last_num[2] && num != last_num[3]) {

            player_order[i] = num;

            last_num[i] = num;

            i++;

            cout << num << endl;

        }

        else {
            cout << "LOOPED" << " " << num << " " << last_num[i] << endl;
        };

    }

}

bool check_game_over(player* point) {
    if (point->turns_taken == 14) {
        GAMEOVER = true;
        return true;
    }

    else { return false; };
};


atomic<int> current_player;
atomic<int> increment = 0;

void shift_player() {
    switch (current_player) {
    case 0:
        pointer = P_1;
        break;

    case 1:
        pointer = P_2;
        break;

    case 2:
        pointer = P_3;
        break;

    case 3:
        pointer = P_4;
        break;
    }
    
    if (check_game_over(pointer) == true) {

    }    
    else {    
        pointer->turns_taken++;
        if (increment >= player_num - 1) {
            
            increment = 0;
        }
        else {
            increment++;
        }
        cout << increment << " " << player_order[increment] << " ";
        current_player = player_order[increment];
        cout << current_player << endl;
    }
}

bool bonus_player_1_card = false;


struct STYLE {
    ImVec4 color_main = ImVec4(255.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 color_active = ImVec4(1.255f, 0.30f, 0.30f, 1.0f);
    ImVec4 color_slider = ImVec4(0.400f, 0.0f, 0.0f, 1.0f);
    ImVec4 color_background = ImVec4(0.300f, 0.0f, 0.0f, 1.0f);
    ImGuiStyle& style_ptr = ImGui::GetStyle();
};


//1024 * 16
//static char player_1_inputs_1[1024 * 16] = "0";

//static char player_1_inputs_2[1024 * 16] = "0";

//static char player_1_inputs_3[1024 * 16] = "0";

//static char player_1_inputs_4[1024 * 16] = "0";

//static char player_1_inputs_5[1024 * 16] = "0";

// Main code
int main(int, char**)
{
    Dice_1 = new DICE;
    Dice_2 = new DICE;
    Dice_3 = new DICE;
    Dice_4 = new DICE;
    Dice_5 = new DICE;
    P_1 = new player;
    P_2 = new player;
    P_3 = new player;
    P_4 = new player;
    const int nums = player_num;
    
    player* players[4] = {P_1, P_2, P_3, P_4};
    /*for (int i = 0; i < player_num; i++) {
        players[player_order[i]] = player();
    }*/
    //cout << players[0].score << endl;
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Yahtzee"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Yahtzee"), WS_OVERLAPPEDWINDOW, 100, 100, MAINWINDOW().WIDTH, MAINWINDOW().HEIGHT, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);    
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_config_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        //Set Colors and Styles
        //cout << ImGui::GetWindowWidth() << " " << ImGui::GetWindowHeight() << endl;
        STYLE().style_ptr.WindowRounding = 12.0f;
        STYLE().style_ptr.FrameRounding = 12.0f;
        STYLE().style_ptr.GrabRounding = 12.0f;
        ImGui::PushStyleColor(ImGuiCol_TitleBg, STYLE().color_background);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, STYLE().color_main);
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, STYLE().color_slider);

        ImGui::PushStyleColor(ImGuiCol_Button, STYLE().color_main);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, STYLE().color_active);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, STYLE().color_slider);

        ImGui::PushStyleColor(ImGuiCol_SliderGrab, STYLE().color_slider);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, STYLE().color_slider);

        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, STYLE().color_main);

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, STYLE().color_main);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, STYLE().color_active);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, STYLE().color_main);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, STYLE().color_active);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, STYLE().color_active);

        ImGui::PushStyleColor(ImGuiCol_ResizeGrip, STYLE().color_main);
        ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, STYLE().color_active);
        ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, STYLE().color_active);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //if (show_demo_window)
        //ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            if (show_config_window == false && GAMEOVER == false) {
                ImGui::SetNextWindowPos(ImVec2(10, 5));
                ImGui::SetNextWindowSize(ImVec2(400, MAINWINDOW().HEIGHT - 100));
                ImGui::Begin("Game", (bool*)false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);                          // Create a window called "Hello, world!" and append into it.
                //ImGuiCol_TitleBgActive
                //ImGuiCol_TitleBg
                //ImGuiCol_TitleBgCollapsed                
                //ImGui::Text("Please select what you would like to do");               // Display some text (you can use a format strings too)
                //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                //ImGui::Checkbox("Settings Window", &show_settings_window);                         
                ImGui::Text("Player");
                ImGui::SameLine();
                ImGui::Text(to_string(current_player + 1).c_str());
                ImGui::SameLine();
                ImGui::Text("turn");                
                if (times_rolled != 3) {
                    if (ImGui::Button("Roll?")) {
                        if (times_rolled != 3 && check_game_over(pointer) == false) {
                            if (Dice_1->is_locked == false) {
                                Dice_1->roll();
                            };

                            if (Dice_2->is_locked == false) {
                                Dice_2->roll();
                            };

                            if (Dice_3->is_locked == false) {
                                Dice_3->roll();
                            };

                            if (Dice_4->is_locked == false) {
                                Dice_4->roll();
                            };

                            if (Dice_5->is_locked == false) {
                                Dice_5->roll();
                            };
                            times_rolled++;
                        }

                        else {
                            times_rolled = times_rolled;                            
                        };
                    }
                }
                //Dice Rendering
                else {
                    if (ImGui::Button("Next Player's turn")) {
                        times_rolled = 0;                        
                        shift_player();
                        Dice_1->reset();
                        Dice_2->reset();
                        Dice_3->reset();
                        Dice_4->reset();
                        Dice_5->reset();
                    }
                }
                ImGui::SameLine();
                thread help_thread_1(HelpMarker, "Roll Dice");
                help_thread_1.join();
                //Dice controls

                //DICE 1

                if (ImGui::Button("Keep Dice 1")) {
                    if (Dice_1->is_locked == true) {
                        Dice_1->lock(false);
                    }

                    else if (Dice_1->is_locked == false) {
                        Dice_1->lock(true);
                    };
                }
                ImGui::SameLine();
                ImGui::Text("Dice 1 ->");
                ImGui::SameLine();
                ImGui::Text(to_string(Dice_1->current_face).c_str());
                ImGui::SameLine();
                if (Dice_1->is_locked == false) {
                    ImGui::Text("Unlocked");
                }

                else if (Dice_1->is_locked == true) {
                    ImGui::Text("Locked");
                }


                //DICE 2
                if (ImGui::Button("Keep Dice 2")) {
                    if (Dice_2->is_locked == true) {
                        Dice_2->lock(false);
                    }

                    else if (Dice_2->is_locked == false) {
                        Dice_2->lock(true);
                    };
                }
                ImGui::SameLine();
                ImGui::Text("Dice 2 ->");
                ImGui::SameLine();
                ImGui::Text(to_string(Dice_2->current_face).c_str());
                ImGui::SameLine();
                if (Dice_2->is_locked == false) {
                    ImGui::Text("Unlocked");
                }

                else if (Dice_2->is_locked == true) {
                    ImGui::Text("Locked");
                }


                //DICE 3
                if (ImGui::Button("Keep Dice 3")) {
                    if (Dice_3->is_locked == true) {
                        Dice_3->lock(false);
                    }

                    else if (Dice_3->is_locked == false) {
                        Dice_3->lock(true);
                    };
                }
                ImGui::SameLine();
                ImGui::Text("Dice 3 ->");
                ImGui::SameLine();
                ImGui::Text(to_string(Dice_3->current_face).c_str());
                ImGui::SameLine();
                if (Dice_3->is_locked == false) {
                    ImGui::Text("Unlocked");
                }

                else if (Dice_3->is_locked == true) {
                    ImGui::Text("Locked");
                }

                //DICE 4
                if (ImGui::Button("Keep Dice 4")) {
                    if (Dice_4->is_locked == true) {
                        Dice_4->lock(false);
                    }

                    else if (Dice_4->is_locked == false) {
                        Dice_4->lock(true);
                    };
                }
                ImGui::SameLine();
                ImGui::Text("Dice 4 ->");
                ImGui::SameLine();
                ImGui::Text(to_string(Dice_4->current_face).c_str());
                ImGui::SameLine();
                if (Dice_4->is_locked == false) {
                    ImGui::Text("Unlocked");
                }

                else if (Dice_4->is_locked == true) {
                    ImGui::Text("Locked");
                }

                //DICE 5
                if (ImGui::Button("Keep Dice 5")) {
                    if (Dice_5->is_locked == true) {
                        Dice_5->lock(false);
                    }

                    else if (Dice_5->is_locked == false) {
                        Dice_5->lock(true);
                    };
                }
                ImGui::SameLine();
                ImGui::Text("Dice 5 ->");
                ImGui::SameLine();
                ImGui::Text(to_string(Dice_5->current_face).c_str());
                ImGui::SameLine();
                if (Dice_5->is_locked == false) {
                    ImGui::Text("Unlocked");
                }

                else if (Dice_5->is_locked == true) {
                    ImGui::Text("Locked");
                };


                if (ImGui::TreeNode("Options")) {
                    if (ImGui::Button("End Game/Reset")) {
                        GAMEOVER = true;
                    }
                    ImGui::NewLine();                   
                    //ImGui::ShowStyleEditor();                    
                    ImGui::TreePop();
                };
                
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
        }


        {
        //Help Window
        ImGui::SetNextWindowPos(ImVec2(400 + 10 + CARD_SIZE().WIDTH, 5));
        ImGui::SetNextWindowSize(ImVec2(HELP_WINDOW().WIDTH, HELP_WINDOW().HEIGHT));
        ImGui::Begin("Help");
        if (ImGui::TreeNode("Rules")) {
            ImGui::TextWrapped("Beginning with the starting player, players will take turns one at a time in clockwise order. The game consists of thirteen rounds and at the end of the thirteenth round then the game will end. (All the categories on the players’ score cards will be completely filled in at that point.)");
            ImGui::TextWrapped("At the start of a turn, the player takes all 5 dice and rolls them. They can then roll some or all of the dice up to two more times, setting aside any dice they’d like to keep and rerolling the rest. The dice can be scored after any of the rolls, but scoring the dice ends the player’s turn. Setting dice aside after one roll does not prevent one or more of them from being rolled again on any subsequent roll if the player so chooses.");
            ImGui::TextWrapped("Each player’s goal is to try and score as high as they can in one of the thirteen categories shown on their score card.");
            ImGui::TextWrapped("To score the dice, the player selects one of the categories on their score card and writes the score into it. Each category can be scored only once per game (except for the Yahtzee category). Categories can be filled in any order. A player must score the dice on their turn even if it turns out that there are no good categories remaining to score in. Once a category is filled it may not be changed.");
            ImGui::TextWrapped("A player may write a score of zero in any category if they have rolled no point-generating results or if they simply choose to do so. For example, a player could put a roll of 2-4-5-6-6 into the Ones category even though it would score zero points.");
            ImGui::TextWrapped("After marking their score on their score card, the player’s turn ends and play proceeds to the player on their left. Or in the case of the game, the next player that has been determined through RNG.");
            ImGui::TreePop();
        };
        if (ImGui::TreeNode("Scoring")) {
            ImGui::Text("Three of a kind -> Three of the same dice.");
            ImGui::NewLine();
            ImGui::Text("Four of a kind -> Four of the same dice.");
            ImGui::NewLine();
            ImGui::TextWrapped("Full House -> Three Dice of the same number. And a pair that are a different number from the three matching.");
            ImGui::NewLine();
            ImGui::TextWrapped("Small (Sm) Straight -> Any four consecutive numbers. (for example, 1-2-3-4)");
            ImGui::NewLine();
            ImGui::TextWrapped("Large (Lg) Straight -> Any five consecutive numbers. (for example, 1-2-3-4-5)");
            ImGui::NewLine();
            ImGui::TextWrapped("Yahtzee (5 of a kind) -> All five dice showing the same number... Any yahtzee after the fist yahtzee is worth 100 points, use the Yahtzee Bonus slider");
            ImGui::NewLine();
            ImGui::Text("Chance -> add up the faces of all dice");
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::BulletText("Special Yahtzee Scoring");
            ImGui::TextWrapped("If a player rolls a Yahtzee on their turn but they have already filled in the Yahtzee category in a previous turn, then special scoring rules apply:\nIf the player has already filled in their Yahtzee box with a score of 50, they receive a Yahtzee bonus of 100 additional points. However, if their\nYahtzee box was previously filled in with a score of zero then they don’t receive the Yahtzee bonus.\n\nThe player then selects another category (other than the Yahtzee category) to score the dice as normal.");
            ImGui::TextWrapped("If the category in the Upper Section that corresponds to the numbers in the Yahtzee is unused, then the player must use that category.");
            ImGui::TextWrapped("If the corresponding box in the Upper Section has been used already then the player may choose to score one of the unused boxes in the Lower Section. In this case, the Yahtzee that the player has rolled acts as a \"Joker\" so that it can be placed in the Full House, Small Straight, and Large Straight categories if the player so wishes, even though it may not meet the standard requirements for those categories.");
            ImGui::TreePop();
        };

        ImGui::End();
        }

        if (GAMEOVER == true) {
            ImGui::Begin("Game Over");            
            int a = P_1->total_score;
            int b = P_2->total_score;
            int c = P_3->total_score;
            int d = P_4->total_score;
            //Finds the largest score
            const int largest = (a > b) ? (a > c) ? (a > d) ? a : d : (c > d) ? c : d : (b > c) ? (b > d) ? b : d : (c > d) ? c : d;
            
            string winner;
            //Finds who has the largest score
            if (largest == a) {
                winner = "Player 1";
            }

            else if (largest == b) {
                winner = "Player 2";

            }

            else if (largest == c) {
                winner = "Player 3";
            }

            else if (largest == d) {
                winner = "Player 4";
            }

            ImGui::Text((winner + " wins!").c_str());
            ImGui::Text("Score -> ");
            ImGui::SameLine();
            ImGui::Text(to_string(largest).c_str());
            ImGui::NewLine();
            ImGui::NewLine();

            ImGui::Text("Player 1 Score ->");
            ImGui::SameLine();
            ImGui::Text(to_string(P_1->total_score).c_str());

            ImGui::Text("Player 2 Score ->");
            ImGui::SameLine();
            ImGui::Text(to_string(P_2->total_score).c_str());

            ImGui::Text("Player 3 Score ->");
            ImGui::SameLine();
            ImGui::Text(to_string(P_3->total_score).c_str());

            ImGui::Text("Player 4 Score ->");
            ImGui::SameLine();
            ImGui::Text(to_string(P_4->total_score).c_str());

            if (Button("Play Again?")) {
                P_1->reset();
                P_2->reset();
                P_3->reset();
                P_4->reset();
                GAMEOVER = false;
                show_config_window = true;
                Dice_1->reset();
                Dice_2->reset();
                Dice_3->reset();
                Dice_4->reset();
                Dice_5->reset();
            }

            ImGui::End();
        }


        //Player 1 score card
        if (show_config_window == false && GAMEOVER == false) {        

            if (current_player == 0) {
                pointer = P_1;
                ImGui::SetNextWindowPos(ImVec2(400 + 10, 5));
                ImGui::SetNextWindowSize(ImVec2(CARD_SIZE().WIDTH, CARD_SIZE().HEIGHT));
                ImGui::Begin("Player 1 score card", (bool*)false, ImGuiWindowFlags_NoResize);
                ImGui::Text("Turns taken");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->turns_taken).c_str());
                if (ImGui::TreeNode("Upper Section")) {
                    //ImGui::Text("Upper Section");
                    
                    HelpMarker("Count and add only Aces");
                    ImGui::SameLine();
                    ImGui::Text("Aces:  ");
                    ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                    ImGui::InputText(" ", pointer->player_inputs_1, IM_ARRAYSIZE(pointer->player_inputs_1));
                    

                    //ImGui::NewLine();
                    HelpMarker("Count and add only Twos");
                    ImGui::SameLine();
                    ImGui::Text("Twos:  ");

                    ImGui::SameLine();

                    ImGui::InputText("  ", pointer->player_inputs_2, IM_ARRAYSIZE(pointer->player_inputs_2));
                    
                   
                    HelpMarker("Count and add only Threes");
                    ImGui::SameLine();
                    //ImGui::InputTextWithHint("Twos", "input", player_1_inputs, IM_ARRAYSIZE(player_1_inputs), ImGuiInputTextFlags_Password);
                    ImGui::Text("Threes:");
                    ImGui::SameLine();
                    ImGui::InputText("   ", pointer->player_inputs_3, IM_ARRAYSIZE(pointer->player_inputs_3));
                    
                    

                    HelpMarker("Count and add only Fours");
                    ImGui::SameLine();
                    ImGui::Text("Fours: ");
                    ImGui::SameLine();
                    ImGui::InputText("    ", pointer->player_inputs_4, IM_ARRAYSIZE(pointer->player_inputs_4));
                    
                    

                    HelpMarker("Count and add only Fives");
                    ImGui::SameLine();
                    ImGui::Text("Fives: ");
                    ImGui::SameLine();
                    ImGui::InputText("     ", pointer->player_inputs_5, IM_ARRAYSIZE(pointer->player_inputs_5));
                    
                    

                    HelpMarker("Count and add only Sixes");
                    ImGui::SameLine();
                    ImGui::Text("Sixes: ");
                    ImGui::SameLine();
                    ImGui::InputText("      ", pointer->player_inputs_6, IM_ARRAYSIZE(pointer->player_inputs_6));
                    
                    


                    ImGui::Text("Total -> ");

                    int s[6];
                    stringstream str_1(pointer->player_inputs_1);
                    stringstream str_2(pointer->player_inputs_2);
                    stringstream str_3(pointer->player_inputs_3);
                    stringstream str_4(pointer->player_inputs_4);
                    stringstream str_5(pointer->player_inputs_5);
                    stringstream str_6(pointer->player_inputs_6);

                    str_1 >> s[0];
                    str_2 >> s[1];
                    str_3 >> s[2];
                    str_4 >> s[3];
                    str_5 >> s[4];
                    str_6 >> s[5];
                    ImGui::SameLine();
                    int total_score_player_1_upper = s[0] + s[1] + s[2] + s[3] + s[4] + s[5];//stoi(player_1_inputs_1) + stoi(player_1_inputs_2) + stoi(player_1_inputs_3) + stoi(player_1_inputs_4) + stoi(player_1_inputs_5);
                    ImGui::Text(to_string(total_score_player_1_upper).c_str());
                    ImGui::SameLine();
                    HelpMarker("Total of upper without bonus");

                    ImGui::Text("Bonus -> ");
                    ImGui::SameLine();
                    if (total_score_player_1_upper >= 63) {
                        ImGui::Text("+35");
                        total_score_player_1_upper += 35;
                    }
                    else {
                        ImGui::Text("0");
                    }
                    ImGui::SameLine();
                    HelpMarker("Bonus Points");


                    ImGui::Text("Grand total (Upper) -> ");
                    ImGui::SameLine();
                    ImGui::Text(to_string(total_score_player_1_upper).c_str());
                    ImGui::SameLine();
                    HelpMarker("Grand Total of Upper");

                    pointer->upper_score = total_score_player_1_upper;
                    ImGui::TreePop();
                };

                if (ImGui::TreeNode("Lower Section")) {
                    //ImGui::Text("Upper Section");
                    HelpMarker("Add Total of all dice");
                    ImGui::SameLine();
                    ImGui::Text("3 of a kind:  ");
                    ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                    ImGui::InputText(" ", pointer->three_of_kind, IM_ARRAYSIZE(pointer->three_of_kind));
                   
                   

                    HelpMarker("Add Total of all dice");
                    ImGui::SameLine();
                    ImGui::Text("4 of a kind:  ");
                    ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                    ImGui::InputText("  ", pointer->four_of_kind, IM_ARRAYSIZE(pointer->four_of_kind));
                    
                    


                    //ImGui::Text("3 of a kind");
                    
                    ImGui::Checkbox("Full House", &pointer->fullhouse);
                    ImGui::SameLine();
                    HelpMarker("Score +25");


                    
                    ImGui::Checkbox("SM Straight", &pointer->sm_straight);
                    ImGui::SameLine();
                    HelpMarker("Score +30");


                    
                    ImGui::Checkbox("LG Straight", &pointer->lg_straight);
                    ImGui::SameLine();
                    HelpMarker("Score +40");
                    
                    ImGui::Checkbox("Yahtzee", &pointer->scored_yahtzee);
                    ImGui::SameLine();
                    HelpMarker("Score +50");
                    HelpMarker("Add Total of all 5 dice");
                    ImGui::SameLine();
                    ImGui::Text("Chance");
                    ImGui::SameLine();
                    ImGui::InputText("        ", pointer->chance, IM_ARRAYSIZE(pointer->chance));
                    
                   
                    HelpMarker("Score +100 each (max 4)");
                    ImGui::SameLine();
                    ImGui::Text("Yahtzee Bonus");
                    ImGui::SameLine();
                    ImGui::SliderInt("Yahtzee Bonus", &pointer->yahtzee_bonus, 0, 4);
                    
                    
                    int total_score_lower = 0;

                    int s[2];
                    stringstream str_1(pointer->three_of_kind);
                    stringstream str_2(pointer->four_of_kind);
                    str_1 >> s[0];
                    str_2 >> s[1];
                    total_score_lower += s[0] + s[1];

                    if (pointer->scored_yahtzee == true) {
                        total_score_lower += 50;
                    };

                    if (pointer->lg_straight == true) {
                        total_score_lower += 40;
                    };

                    if (pointer->sm_straight == true) {
                        total_score_lower += 30;
                    };

                    if (pointer->fullhouse == true) {
                        total_score_lower += 25;
                    };

                    if (pointer->yahtzee_bonus != 0) {
                        for (int i = 0; i < pointer->yahtzee_bonus; i++) {
                            total_score_lower += 100;
                        };
                    };

                    pointer->lower_score = total_score_lower;
                    ImGui::Text("Grand Total (Lower) -> ");
                    ImGui::SameLine();
                    ImGui::Text(to_string(pointer->lower_score).c_str());
                    ImGui::SameLine();
                    HelpMarker("Grand Total of lower");
                    pointer->total_score = pointer->lower_score + pointer->upper_score;
                    ImGui::Text("GRAND TOTAL -> ");
                    ImGui::SameLine();
                    ImGui::Text(to_string(pointer->total_score).c_str());
                    ImGui::SameLine();
                    HelpMarker("Grand Total of both upper and lower");
                    ImGui::TreePop();
                };

                ImGui::End();
            }
            //Player 2
            else if (current_player == 1) {
            pointer = P_2;
            ImGui::SetNextWindowPos(ImVec2(400 + 10, 5));
            ImGui::SetNextWindowSize(ImVec2(CARD_SIZE().WIDTH, CARD_SIZE().HEIGHT));
            ImGui::Begin("Player 2 score card", (bool*)false, ImGuiWindowFlags_NoResize);
            ImGui::Text("Turns taken");
            ImGui::SameLine();
            ImGui::Text(to_string(pointer->turns_taken).c_str());
            if (ImGui::TreeNode("Upper Section")) {
                //ImGui::Text("Upper Section");

                HelpMarker("Count and add only Aces");
                ImGui::SameLine();
                ImGui::Text("Aces:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->player_inputs_1, IM_ARRAYSIZE(pointer->player_inputs_1));


                //ImGui::NewLine();
                HelpMarker("Count and add only Twos");
                ImGui::SameLine();
                ImGui::Text("Twos:  ");

                ImGui::SameLine();

                ImGui::InputText("  ", pointer->player_inputs_2, IM_ARRAYSIZE(pointer->player_inputs_2));


                HelpMarker("Count and add only Threes");
                ImGui::SameLine();
                //ImGui::InputTextWithHint("Twos", "input", player_1_inputs, IM_ARRAYSIZE(player_1_inputs), ImGuiInputTextFlags_Password);
                ImGui::Text("Threes:");
                ImGui::SameLine();
                ImGui::InputText("   ", pointer->player_inputs_3, IM_ARRAYSIZE(pointer->player_inputs_3));



                HelpMarker("Count and add only Fours");
                ImGui::SameLine();
                ImGui::Text("Fours: ");
                ImGui::SameLine();
                ImGui::InputText("    ", pointer->player_inputs_4, IM_ARRAYSIZE(pointer->player_inputs_4));



                HelpMarker("Count and add only Fives");
                ImGui::SameLine();
                ImGui::Text("Fives: ");
                ImGui::SameLine();
                ImGui::InputText("     ", pointer->player_inputs_5, IM_ARRAYSIZE(pointer->player_inputs_5));



                HelpMarker("Count and add only Sixes");
                ImGui::SameLine();
                ImGui::Text("Sixes: ");
                ImGui::SameLine();
                ImGui::InputText("      ", pointer->player_inputs_6, IM_ARRAYSIZE(pointer->player_inputs_6));




                ImGui::Text("Total -> ");

                int s[6];
                stringstream str_1(pointer->player_inputs_1);
                stringstream str_2(pointer->player_inputs_2);
                stringstream str_3(pointer->player_inputs_3);
                stringstream str_4(pointer->player_inputs_4);
                stringstream str_5(pointer->player_inputs_5);
                stringstream str_6(pointer->player_inputs_6);

                str_1 >> s[0];
                str_2 >> s[1];
                str_3 >> s[2];
                str_4 >> s[3];
                str_5 >> s[4];
                str_6 >> s[5];
                ImGui::SameLine();
                int total_score_player_1_upper = s[0] + s[1] + s[2] + s[3] + s[4] + s[5];//stoi(player_1_inputs_1) + stoi(player_1_inputs_2) + stoi(player_1_inputs_3) + stoi(player_1_inputs_4) + stoi(player_1_inputs_5);
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Total of upper without bonus");

                ImGui::Text("Bonus -> ");
                ImGui::SameLine();
                if (total_score_player_1_upper >= 63) {
                    ImGui::Text("+35");
                    total_score_player_1_upper += 35;
                }
                else {
                    ImGui::Text("0");
                }
                ImGui::SameLine();
                HelpMarker("Bonus Points");


                ImGui::Text("Grand total (Upper) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of Upper");

                pointer->upper_score = total_score_player_1_upper;
                ImGui::TreePop();
            };

            if (ImGui::TreeNode("Lower Section")) {
                //ImGui::Text("Upper Section");
                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("3 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->three_of_kind, IM_ARRAYSIZE(pointer->three_of_kind));



                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("4 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText("  ", pointer->four_of_kind, IM_ARRAYSIZE(pointer->four_of_kind));




                //ImGui::Text("3 of a kind");

                ImGui::Checkbox("Full House", &pointer->fullhouse);
                ImGui::SameLine();
                HelpMarker("Score +25");



                ImGui::Checkbox("SM Straight", &pointer->sm_straight);
                ImGui::SameLine();
                HelpMarker("Score +30");



                ImGui::Checkbox("LG Straight", &pointer->lg_straight);
                ImGui::SameLine();
                HelpMarker("Score +40");

                ImGui::Checkbox("Yahtzee", &pointer->scored_yahtzee);
                ImGui::SameLine();
                HelpMarker("Score +50");
                HelpMarker("Add Total of all 5 dice");
                ImGui::SameLine();
                ImGui::Text("Chance");
                ImGui::SameLine();
                ImGui::InputText("        ", pointer->chance, IM_ARRAYSIZE(pointer->chance));


                HelpMarker("Score +100 each (max 4)");
                ImGui::SameLine();
                ImGui::Text("Yahtzee Bonus");
                ImGui::SameLine();
                ImGui::SliderInt("Yahtzee Bonus", &pointer->yahtzee_bonus, 0, 4);


                int total_score_lower = 0;

                int s[2];
                stringstream str_1(pointer->three_of_kind);
                stringstream str_2(pointer->four_of_kind);
                str_1 >> s[0];
                str_2 >> s[1];
                total_score_lower += s[0] + s[1];

                if (pointer->scored_yahtzee == true) {
                    total_score_lower += 50;
                };

                if (pointer->lg_straight == true) {
                    total_score_lower += 40;
                };

                if (pointer->sm_straight == true) {
                    total_score_lower += 30;
                };

                if (pointer->fullhouse == true) {
                    total_score_lower += 25;
                };

                if (pointer->yahtzee_bonus != 0) {
                    for (int i = 0; i < pointer->yahtzee_bonus; i++) {
                        total_score_lower += 100;
                    };
                };

                pointer->lower_score = total_score_lower;
                ImGui::Text("Grand Total (Lower) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->lower_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of lower");
                pointer->total_score = pointer->lower_score + pointer->upper_score;
                ImGui::Text("GRAND TOTAL -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->total_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of both upper and lower");
                ImGui::TreePop();
            };

            ImGui::End();
            }
            //Player 3
            else if (current_player == 2) {
            pointer = P_3;
            ImGui::SetNextWindowPos(ImVec2(400 + 10, 5));
            ImGui::SetNextWindowSize(ImVec2(CARD_SIZE().WIDTH, CARD_SIZE().HEIGHT));
            ImGui::Begin("Player 3 score card", (bool*)false, ImGuiWindowFlags_NoResize);
            ImGui::Text("Turns taken");
            ImGui::SameLine();
            ImGui::Text(to_string(pointer->turns_taken).c_str());
            if (ImGui::TreeNode("Upper Section")) {
                //ImGui::Text("Upper Section");

                HelpMarker("Count and add only Aces");
                ImGui::SameLine();
                ImGui::Text("Aces:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->player_inputs_1, IM_ARRAYSIZE(pointer->player_inputs_1));


                //ImGui::NewLine();
                HelpMarker("Count and add only Twos");
                ImGui::SameLine();
                ImGui::Text("Twos:  ");

                ImGui::SameLine();

                ImGui::InputText("  ", pointer->player_inputs_2, IM_ARRAYSIZE(pointer->player_inputs_2));


                HelpMarker("Count and add only Threes");
                ImGui::SameLine();
                //ImGui::InputTextWithHint("Twos", "input", player_1_inputs, IM_ARRAYSIZE(player_1_inputs), ImGuiInputTextFlags_Password);
                ImGui::Text("Threes:");
                ImGui::SameLine();
                ImGui::InputText("   ", pointer->player_inputs_3, IM_ARRAYSIZE(pointer->player_inputs_3));



                HelpMarker("Count and add only Fours");
                ImGui::SameLine();
                ImGui::Text("Fours: ");
                ImGui::SameLine();
                ImGui::InputText("    ", pointer->player_inputs_4, IM_ARRAYSIZE(pointer->player_inputs_4));



                HelpMarker("Count and add only Fives");
                ImGui::SameLine();
                ImGui::Text("Fives: ");
                ImGui::SameLine();
                ImGui::InputText("     ", pointer->player_inputs_5, IM_ARRAYSIZE(pointer->player_inputs_5));



                HelpMarker("Count and add only Sixes");
                ImGui::SameLine();
                ImGui::Text("Sixes: ");
                ImGui::SameLine();
                ImGui::InputText("      ", pointer->player_inputs_6, IM_ARRAYSIZE(pointer->player_inputs_6));




                ImGui::Text("Total -> ");

                int s[6];
                stringstream str_1(pointer->player_inputs_1);
                stringstream str_2(pointer->player_inputs_2);
                stringstream str_3(pointer->player_inputs_3);
                stringstream str_4(pointer->player_inputs_4);
                stringstream str_5(pointer->player_inputs_5);
                stringstream str_6(pointer->player_inputs_6);

                str_1 >> s[0];
                str_2 >> s[1];
                str_3 >> s[2];
                str_4 >> s[3];
                str_5 >> s[4];
                str_6 >> s[5];
                ImGui::SameLine();
                int total_score_player_1_upper = s[0] + s[1] + s[2] + s[3] + s[4] + s[5];//stoi(player_1_inputs_1) + stoi(player_1_inputs_2) + stoi(player_1_inputs_3) + stoi(player_1_inputs_4) + stoi(player_1_inputs_5);
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Total of upper without bonus");

                ImGui::Text("Bonus -> ");
                ImGui::SameLine();
                if (total_score_player_1_upper >= 63) {
                    ImGui::Text("+35");
                    total_score_player_1_upper += 35;
                }
                else {
                    ImGui::Text("0");
                }
                ImGui::SameLine();
                HelpMarker("Bonus Points");


                ImGui::Text("Grand total (Upper) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of Upper");

                pointer->upper_score = total_score_player_1_upper;
                ImGui::TreePop();
            };

            if (ImGui::TreeNode("Lower Section")) {
                //ImGui::Text("Upper Section");
                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("3 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->three_of_kind, IM_ARRAYSIZE(pointer->three_of_kind));



                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("4 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText("  ", pointer->four_of_kind, IM_ARRAYSIZE(pointer->four_of_kind));




                //ImGui::Text("3 of a kind");

                ImGui::Checkbox("Full House", &pointer->fullhouse);
                ImGui::SameLine();
                HelpMarker("Score +25");



                ImGui::Checkbox("SM Straight", &pointer->sm_straight);
                ImGui::SameLine();
                HelpMarker("Score +30");



                ImGui::Checkbox("LG Straight", &pointer->lg_straight);
                ImGui::SameLine();
                HelpMarker("Score +40");

                ImGui::Checkbox("Yahtzee", &pointer->scored_yahtzee);
                ImGui::SameLine();
                HelpMarker("Score +50");
                HelpMarker("Add Total of all 5 dice");
                ImGui::SameLine();
                ImGui::Text("Chance");
                ImGui::SameLine();
                ImGui::InputText("        ", pointer->chance, IM_ARRAYSIZE(pointer->chance));


                HelpMarker("Score +100 each (max 4)");
                ImGui::SameLine();
                ImGui::Text("Yahtzee Bonus");
                ImGui::SameLine();
                ImGui::SliderInt("Yahtzee Bonus", &pointer->yahtzee_bonus, 0, 4);


                int total_score_lower = 0;

                int s[2];
                stringstream str_1(pointer->three_of_kind);
                stringstream str_2(pointer->four_of_kind);
                str_1 >> s[0];
                str_2 >> s[1];
                total_score_lower += s[0] + s[1];

                if (pointer->scored_yahtzee == true) {
                    total_score_lower += 50;
                };

                if (pointer->lg_straight == true) {
                    total_score_lower += 40;
                };

                if (pointer->sm_straight == true) {
                    total_score_lower += 30;
                };

                if (pointer->fullhouse == true) {
                    total_score_lower += 25;
                };

                if (pointer->yahtzee_bonus != 0) {
                    for (int i = 0; i < pointer->yahtzee_bonus; i++) {
                        total_score_lower += 100;
                    };
                };

                pointer->lower_score = total_score_lower;
                ImGui::Text("Grand Total (Lower) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->lower_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of lower");
                pointer->total_score = pointer->lower_score + pointer->upper_score;
                ImGui::Text("GRAND TOTAL -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->total_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of both upper and lower");
                ImGui::TreePop();
            };

            ImGui::End();
            }
            //Player 4
            else if (current_player == 3) {
            pointer = P_4;
            ImGui::SetNextWindowPos(ImVec2(400 + 10, 5));
            ImGui::SetNextWindowSize(ImVec2(CARD_SIZE().WIDTH, CARD_SIZE().HEIGHT));
            ImGui::Begin("Player 4 score card", (bool*)false, ImGuiWindowFlags_NoResize);
            ImGui::Text("Turns taken");
            ImGui::SameLine();
            ImGui::Text(to_string(pointer->turns_taken).c_str());
            if (ImGui::TreeNode("Upper Section")) {
                //ImGui::Text("Upper Section");

                HelpMarker("Count and add only Aces");
                ImGui::SameLine();
                ImGui::Text("Aces:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->player_inputs_1, IM_ARRAYSIZE(pointer->player_inputs_1));


                //ImGui::NewLine();
                HelpMarker("Count and add only Twos");
                ImGui::SameLine();
                ImGui::Text("Twos:  ");

                ImGui::SameLine();

                ImGui::InputText("  ", pointer->player_inputs_2, IM_ARRAYSIZE(pointer->player_inputs_2));


                HelpMarker("Count and add only Threes");
                ImGui::SameLine();
                //ImGui::InputTextWithHint("Twos", "input", player_1_inputs, IM_ARRAYSIZE(player_1_inputs), ImGuiInputTextFlags_Password);
                ImGui::Text("Threes:");
                ImGui::SameLine();
                ImGui::InputText("   ", pointer->player_inputs_3, IM_ARRAYSIZE(pointer->player_inputs_3));



                HelpMarker("Count and add only Fours");
                ImGui::SameLine();
                ImGui::Text("Fours: ");
                ImGui::SameLine();
                ImGui::InputText("    ", pointer->player_inputs_4, IM_ARRAYSIZE(pointer->player_inputs_4));



                HelpMarker("Count and add only Fives");
                ImGui::SameLine();
                ImGui::Text("Fives: ");
                ImGui::SameLine();
                ImGui::InputText("     ", pointer->player_inputs_5, IM_ARRAYSIZE(pointer->player_inputs_5));



                HelpMarker("Count and add only Sixes");
                ImGui::SameLine();
                ImGui::Text("Sixes: ");
                ImGui::SameLine();
                ImGui::InputText("      ", pointer->player_inputs_6, IM_ARRAYSIZE(pointer->player_inputs_6));




                ImGui::Text("Total -> ");

                int s[6];
                stringstream str_1(pointer->player_inputs_1);
                stringstream str_2(pointer->player_inputs_2);
                stringstream str_3(pointer->player_inputs_3);
                stringstream str_4(pointer->player_inputs_4);
                stringstream str_5(pointer->player_inputs_5);
                stringstream str_6(pointer->player_inputs_6);

                str_1 >> s[0];
                str_2 >> s[1];
                str_3 >> s[2];
                str_4 >> s[3];
                str_5 >> s[4];
                str_6 >> s[5];
                ImGui::SameLine();
                int total_score_player_1_upper = s[0] + s[1] + s[2] + s[3] + s[4] + s[5];//stoi(player_1_inputs_1) + stoi(player_1_inputs_2) + stoi(player_1_inputs_3) + stoi(player_1_inputs_4) + stoi(player_1_inputs_5);
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Total of upper without bonus");

                ImGui::Text("Bonus -> ");
                ImGui::SameLine();
                if (total_score_player_1_upper >= 63) {
                    ImGui::Text("+35");
                    total_score_player_1_upper += 35;
                }
                else {
                    ImGui::Text("0");
                }
                ImGui::SameLine();
                HelpMarker("Bonus Points");


                ImGui::Text("Grand total (Upper) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(total_score_player_1_upper).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of Upper");

                pointer->upper_score = total_score_player_1_upper;
                ImGui::TreePop();
            };

            if (ImGui::TreeNode("Lower Section")) {
                //ImGui::Text("Upper Section");
                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("3 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText(" ", pointer->three_of_kind, IM_ARRAYSIZE(pointer->three_of_kind));



                HelpMarker("Add Total of all dice");
                ImGui::SameLine();
                ImGui::Text("4 of a kind:  ");
                ImGui::SameLine();//((int)(sizeof(player_1_inputs) / sizeof(*(player_1_inputs))))            
                ImGui::InputText("  ", pointer->four_of_kind, IM_ARRAYSIZE(pointer->four_of_kind));




                //ImGui::Text("3 of a kind");

                ImGui::Checkbox("Full House", &pointer->fullhouse);
                ImGui::SameLine();
                HelpMarker("Score +25");



                ImGui::Checkbox("SM Straight", &pointer->sm_straight);
                ImGui::SameLine();
                HelpMarker("Score +30");



                ImGui::Checkbox("LG Straight", &pointer->lg_straight);
                ImGui::SameLine();
                HelpMarker("Score +40");

                ImGui::Checkbox("Yahtzee", &pointer->scored_yahtzee);
                ImGui::SameLine();
                HelpMarker("Score +50");
                HelpMarker("Add Total of all 5 dice");
                ImGui::SameLine();
                ImGui::Text("Chance");
                ImGui::SameLine();
                ImGui::InputText("        ", pointer->chance, IM_ARRAYSIZE(pointer->chance));


                HelpMarker("Score +100 each (max 4)");
                ImGui::SameLine();
                ImGui::Text("Yahtzee Bonus");
                ImGui::SameLine();
                ImGui::SliderInt("Yahtzee Bonus", &pointer->yahtzee_bonus, 0, 4);


                int total_score_lower = 0;

                int s[2];
                stringstream str_1(pointer->three_of_kind);
                stringstream str_2(pointer->four_of_kind);
                str_1 >> s[0];
                str_2 >> s[1];
                total_score_lower += s[0] + s[1];

                if (pointer->scored_yahtzee == true) {
                    total_score_lower += 50;
                };

                if (pointer->lg_straight == true) {
                    total_score_lower += 40;
                };

                if (pointer->sm_straight == true) {
                    total_score_lower += 30;
                };

                if (pointer->fullhouse == true) {
                    total_score_lower += 25;
                };

                if (pointer->yahtzee_bonus != 0) {
                    for (int i = 0; i < pointer->yahtzee_bonus; i++) {
                        total_score_lower += 100;
                    };
                };

                pointer->lower_score = total_score_lower;
                ImGui::Text("Grand Total (Lower) -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->lower_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of lower");
                pointer->total_score = pointer->lower_score + pointer->upper_score;
                ImGui::Text("GRAND TOTAL -> ");
                ImGui::SameLine();
                ImGui::Text(to_string(pointer->total_score).c_str());
                ImGui::SameLine();
                HelpMarker("Grand Total of both upper and lower");
                ImGui::TreePop();
            };

            ImGui::End();
            }
        }


        // 3. Show another simple window.
        if (show_config_window)
        {
           
            ImGui::Begin("Save", (bool*)false, ImGuiWindowFlags_NoCollapse);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            
            ImGui::Text("How many players?");
            ImGui::SameLine();
            ImGui::SliderInt("   ", &player_num, 1, 4);
            if (ImGui::Button("Start?")) {


                thread th(turn_cycle);

                th.join();
                //turn_cycle();

                current_player = player_order[increment];

                show_config_window = false;
            }

            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 0.0f), (int)(clear_color.z * 0.0f), (int)(clear_color.w * 1.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}


void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}