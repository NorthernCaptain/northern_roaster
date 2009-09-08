#include "game_wrapper.hxx"
#include "screen_intro.hxx"

static Uint8    game_color_names[MAX_GAME_COLORS][4] = 
{
    /* 0  "Red"             */ { 255,  0,  0, 255},  
    /* 1  "Green"           */ {   0,255,  0, 255}, 
    /* 2  "Blue"            */ {   0,  0,255, 255}, 
    /* 3  "Yellow"          */ { 255,255,  0, 255},
    /* 4  "Magenta"         */ { 255,  0,255, 255}, 
    /* 5  "Cyan"            */ {  68, 93,112, 255}, 
    /* 6  "FireBrick"       */ { 178, 34, 34, 255}, 
    /* 7  "SpringGreen",    */ {   0,255,127, 255}, 
    /* 8  "CornflowerBlue"  */ { 100,149,237, 255},  
    /* 9  "Khaki"           */ { 240,230,140, 255},
    /*10  "Plum"            */ { 221,160,221, 255}, 
    /*11  "Violet"          */ { 238,130,238, 255},
    /*12  "DarkTurquoise"   */ {   0,206,209, 255}, 
    /*13  "Gold"            */ { 255,215,  0, 255},
    /*14  "Orchid"          */ { 218,112,214, 255}, 
    /*15  "Turquoise"       */ {  64,224,208, 255},
    /*16  "Orange"          */ { 255,165,  0, 255},
    /*17  "OrangeRed"       */ { 255, 69,  0, 255}, 
    /*18  "VioletRed"       */ { 208, 32,144, 255}, 
    /*19  "BlueViolet"      */ { 138, 43,226, 255},
    /*20  "SeaGreen"        */ {  46,139, 87, 255}, 
    /*21  "Pink"            */ { 255,192,203, 255}, 
    /*22  "ForestGreen"     */ {  34,139, 34, 255},  
    /*23  "SkyBlue"         */ { 135,206,235, 255},
    /*24  "Coral"           */ { 168,107, 47, 255}, 
    /*25  "Wheat"           */ { 245,222,179, 255},
    /*26  "GoldenRod"       */ { 218,165, 32, 255},
    /*27  "IndianRed"       */ { 205, 92, 92, 255},
    /*28  "SpringGreen"     */ {   0,255,127, 255}, 
    /*29  "CornflowerBlue"  */ { 100,149,237, 255}, 
    /*30  "Thistle"         */ { 216,191,216, 255}, 
    /*31  "Aquamarine"      */ { 127,255,212, 255},
    /*32  "CadetBlue"       */ {  95,158,160, 255},
    /*33  "LightSteelBlue"  */ { 176,196,222, 255}, 
    /*34  "NavyBlue"        */ {   0,  0,128, 255},
    /*35  "SteelBlue"       */ {  70,130,180, 255}, 
    /*36  "YellowGreen"     */ { 154,205, 50, 255}, 
    /*37  "DarkViolet"      */ { 148,  0,211, 255}, 
    /*38  "MediumSeaGreen"  */ {  60,179,113, 255}, 
    /*39  "DarkSlateGray"   */ {  47, 79, 79, 255},
    /*40  "LightGray"       */ { 211,211,211, 255}, 
    /*41  "MediumVioletRed" */ { 199, 21,133, 255},
    /*42  "Sienna"          */ { 160, 82, 45, 255}, 
    /*43  "MediumAquamarine"*/ { 102,205,170, 255},
    /*44  "MediumBlue"      */ {   0,  0,205, 255},
    /*45  "Navy"            */ {   0,  0,128, 255},
    /*46  "DarkOliveGreen"  */ {  85,107, 47, 255}, 
    /*47  "DarkGreen"       */ {   0,100,  0, 255},
    /*48  "DimGray"         */ { 105,105,105, 255}, 
    /*49  "Tan"             */ { 210,180,140, 255},
    /*50  "MediumTurquoise" */ {  72,209,204, 255},
    /*51  "DarkSlateBlue"   */ {  72, 61,139, 255}, 
    /*52  "Maroon"          */ { 176, 48, 96, 255}, 
    /*53  "Gray"            */ { 190,190,190, 255},
    /*54  "#303030"         */ { 150,150,150, 255},
    
    /*55  "Black"           */ {   0,  0,  0, 255},
    /*56  "#00aa00"         */ {   0,0xaa, 0, 255},
    /*57  "#ffffff"         */ {0xff,0xff, 0xff, 255},
    /*58  "#aa0000"         */ {0xaa,  0,  0, 255},
    /*59  "#ffffff"         */ {0x20,0xff,0x20, 100},
    /*60  "#ffff00"         */ {0xff,0xff,0x00, 255},
    /*61  "#eeee00"         */ {0x10,0x10,0x30, 255}
};

GameWrapper::GameWrapper() : Game()
{
    load_images();
    init_colors();
}

GameWrapper::~GameWrapper()
{
}


void GameWrapper::init_gameplay()
{
    screen_login=Screen_LoginPtr(new Screen_Login(scan_task, this, "login"));
    screen_intro=Screen_IntroPtr(new Screen_Intro(scan_task, this, "intro"));
    screen_intro2=Screen_Intro2Ptr(new Screen_Intro2(scan_task, this, "intro2"));
    screen_finish=Screen_FinishPtr(new Screen_Finish(scan_task, this, "finish"));
    screen_confirm=Screen_ConfirmPtr(new Screen_Confirm(scan_task, this, "confirm"));
    screen_intro->set_prev_screen(screen_login.get());
    screen_intro2->set_prev_screen(screen_intro.get());
    screen_confirm->set_prev_screen(screen_intro2.get());
    scene_add(screen_login.get());
    screen_main=Screen_MainPtr(new Screen_Main(scan_task, this, "main"));
    screen_login->finished_signal.connect(SigC::slot(*screen_intro, &Screen_Intro::show));
    screen_intro->finished_signal.connect(SigC::slot(*screen_intro2, &Screen_Intro2::show));
    screen_intro2->finished_signal.connect(SigC::slot(*screen_confirm, &Screen_Confirm::show));
    screen_confirm->finished_signal.connect(SigC::slot(*screen_main, &Screen_Main::show));
    screen_main->finished_signal.connect(SigC::slot(*screen_finish, &Screen_Finish::show));
    screen_finish->finished_signal.connect(SigC::slot(*screen_intro, &Screen_Intro::show));
    screen_main->set_prev_screen(screen_intro.get()); //??? intro ???

    scan_task->set_notifier(new GameNotifier(1));
}

void GameWrapper::load_images()
{
    bg=image_bank[imIntroBG]=load_image("bg_params.png");    
    bg=image_bank[imIntro2BG]=load_image("bg_params2.png");    
    bg=image_bank[imFinishBG]=load_image("bg_finish.png");    
    bg=image_bank[imConfirmBG]=load_image("bg_confirm.png");    
    bg=image_bank[imLoginBG]=load_image("bg_login.png");    
    image_bank[imFont1]=load_image("font_01.png");    
    fonts[imFont1]=LoadFontFromSurface(image_bank[imFont1]);
    image_bank[imFont2]=load_image("font_02.png");    
    fonts[imFont2]=LoadFontFromSurface(image_bank[imFont2]);
    image_bank[imFont3]=load_image("font_03.png");    
    fonts[imFont3]=LoadFontFromSurface(image_bank[imFont3]);
    image_bank[imFont4]=load_image("font_04.png");    
    fonts[imFont4]=LoadFontFromSurface(image_bank[imFont4]);
    image_bank[imFont5]=load_image("font_05.png");    
    fonts[imFont5]=LoadFontFromSurface(image_bank[imFont5]);
    image_bank[imFont6]=load_image("font_06.png");    
    fonts[imFont6]=LoadFontFromSurface(image_bank[imFont6]);
    image_bank[imBoardBG]=load_image("bg_roast.png");
    image_bank[imDigits]=load_image("digits_1.png");    
}

//===========================================================================
/// local init_colors()
///     allocate all nessesary colors
/// tags SDLWellEngine
void GameWrapper::init_colors()
{
    int i;
    for(i=0;i<MAX_GAME_COLORS;i++)
    {
	game_colors[i]=SDL_MapRGBA(Game::screen->format, 
				   game_color_names[i][0],
				   game_color_names[i][1],
				   game_color_names[i][2], 255);
	game_RGBA[i*4+0]=game_color_names[i][0];
	game_RGBA[i*4+1]=game_color_names[i][1];
	game_RGBA[i*4+2]=game_color_names[i][2];
	game_RGBA[i*4+3]=game_color_names[i][3];
    }
}

