#include <iostream>
#include <stdlib.h>
#include <allegro.h>

#include "VectorField2D.h"
#include "VectorField2DVisualizer.h"
#include "ColorScale.h"

using namespace std;

#define FIELD_DIM	5

BITMAP *buffer;
vector2d field[FIELD_DIM*FIELD_DIM];

int method = 1, scale = 0, critical = 0;

char info[7][50] = {"This demo program creates a random vector",
		    "field on a regular 5x5 grid, visualizing",
		    "it using one of the listed techniques.",
		    "The magnitude of the vectors can be color",
		    "coded with either color scale and critical",
		    "points will be shown if desired.",
		    "(computations may be very time intensive)"};

int d_choose_tech_proc(int msg, DIALOG *d, int c)
{
	if (msg == MSG_CLICK || msg == MSG_KEY)
		method = (int)d->dp2;
	
	return d_radio_proc(msg, d, c);
}

int d_choose_scale_proc(int msg, DIALOG *d, int c)
{
	if (msg == MSG_CLICK || msg == MSG_KEY)
		scale = (int)d->dp2;
	
	return d_radio_proc(msg, d, c);
}

DIALOG dialog[] =
{
   // (dialog proc)       (x)  (y)   (w)   (h) (fg)(bg) (key) (flags)     (d1) (d2)    (dp)                   (dp2) (dp3)
   { d_clear_proc,         0,   0,    0,    0,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { d_shadow_box_proc,   10,  10,  360,   28,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { d_box_proc,          10, 145,  360,  110,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { d_box_proc,          10, 270,  360,  110,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { d_box_proc,          10, 395,  360,   62,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { d_text_proc,         45,  20,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)"Image-based flow visualization demo", NULL, NULL  },
   { d_text_proc,         20,  60,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[0],          NULL, NULL  },
   { d_text_proc,         20,  70,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[1],          NULL, NULL  },
   { d_text_proc,         20,  80,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[2],          NULL, NULL  },
   { d_text_proc,         20,  90,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[3],          NULL, NULL  },
   { d_text_proc,         20, 100,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[4],          NULL, NULL  },
   { d_text_proc,         20, 110,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[5],          NULL, NULL  },
   { d_text_proc,         20, 120,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)info[6],          NULL, NULL  },
   { d_text_proc,         20, 160,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)"Choose visualization technique:", NULL, NULL  },
   { d_choose_tech_proc,  40, 180,  280,   10,   0,  0,  'a', D_SELECTED,   1,   0,   (void *)" Modified &Arrow Plot", (void *)1, NULL  },
   { d_choose_tech_proc,  40, 192,  280,   10,   0,  0,  'i',      0,       1,   0,   (void *)" &Integrate And Draw",  (void *)2, NULL  },
   { d_choose_tech_proc,  40, 204,  280,   10,   0,  0,  'l',      0,       1,   0,   (void *)" &Line Integral Convolution (LIC)", (void *)3, NULL  },
   { d_choose_tech_proc,  40, 216,  280,   10,   0,  0,  's',      0,       1,   0,   (void *)" &Spot Noise", (void *)4, NULL  },
   { d_choose_tech_proc,  40, 228,  280,   10,   0,  0,  'd',      0,       1,   0,   (void *)" Texture A&dvection", (void *)5, NULL  },
   { d_text_proc,         20, 285,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)"Choose color scale:", NULL, NULL  },
   { d_choose_scale_proc, 40, 305,  280,   10,   0,  0,  'r',      0,       2,   0,   (void *)" &Rainbow Scale", (void *)1, NULL  },
   { d_choose_scale_proc, 40, 317,  280,   10,   0,  0,  'h',      0,       2,   0,   (void *)" &Heated Body Scale", (void *)2, NULL  },
   { d_choose_scale_proc, 40, 329,  280,   10,   0,  0,  't',      0,       2,   0,   (void *)" &Temperature Scale", (void *)3, NULL  },
   { d_choose_scale_proc, 40, 341,  280,   10,   0,  0,  'b',      0,       2,   0,   (void *)" &Blue-Yellow Scale", (void *)4, NULL  },
   { d_choose_scale_proc, 40, 353,  280,   10,   0,  0,  'n', D_SELECTED,   2,   0,   (void *)" &None",    (void *)0, NULL  },
   { d_text_proc,         20, 410,    0,    0,   0,  0,    0,      0,       0,   0,   (void *)"Critical points:", NULL, NULL  },
   { d_check_proc,        40, 430,  280,   10,   0,  0,  'c',      0,       1,   0,   (void *)" Show &Critical Points", NULL, NULL  },
   { d_button_proc,       40, 475,   75,   20,   0,  0,    0,   D_EXIT,     0,   0,   (void *)"Run",            NULL, NULL  },
   { d_button_proc,      260, 475,   75,   20,   0,  0,    0,   D_EXIT,     0,   0,   (void *)"Quit",           NULL, NULL  },
   { d_yield_proc,         0,   0,    0,    0,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  },
   { NULL,                 0,   0,    0,    0,   0,  0,    0,      0,       0,   0,   NULL,                   NULL, NULL  }
};

void create_field()
{
	// create vectors on grid with component values from -5 to +5
	for (int i=0; i<FIELD_DIM*FIELD_DIM; i++)
	{
		field[i].x = -5 + rand() % 11;
		field[i].y = -5 + rand() % 11;
	}
}

void init()
{
	allegro_init();

	install_keyboard();
	install_mouse();
	install_timer();

	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 380, 512, 0, 0);

	buffer = create_bitmap(512, 512);

	srand(time(NULL));
	create_field();

	gui_fg_color = makecol(0, 0, 0);
   	gui_mg_color = makecol(128, 128, 128);
   	gui_bg_color = makecol(255, 255, 255);
   	set_dialog_color(dialog, gui_fg_color, gui_bg_color);

	// quit button pressed and exited dialog
	if (do_dialog(dialog, -1) == 28)
	{
		allegro_exit();
		exit(0);
	} // otherwise run button has quit dialog, so go on execution

	// critical points check box enabled, if flag set
	critical = (dialog[26].flags & D_SELECTED);

	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 512, 512, 0, 0);
}

int main()
{
	init();

	ColorScale rainbow(5);
	rainbow.AddPoint(0.5, 0, 0, 1);
	rainbow.AddPoint(1.5, 0, 1, 1);
	rainbow.AddPoint(3.0, 0, 1, 0);
	rainbow.AddPoint(4.5, 1, 1, 0);
	rainbow.AddPoint(6.0, 1, 0, 0);

	ColorScale heat(4);
	heat.AddPoint(1.0, 0, 0, 0);
	heat.AddPoint(3.0, 1, 0, 0);
	heat.AddPoint(5.0, 1, 1, 0);
	heat.AddPoint(7.0, 1, 1, 1);

	ColorScale temp(2);
	temp.AddPoint(1.0, 0, 0, 1);
	temp.AddPoint(6.0, 1, 0, 0);

	ColorScale bluyel(2);
	bluyel.AddPoint(1.0, 0, 0, 1);
	bluyel.AddPoint(6.0, 1, 1, 0);

	VectorField2D myField(field, 5, 5, 1.0f, 1.0f);

	VectorField2DVisualizer myVisualizer(myField, buffer);

	switch (method)
	{
		case 1:
			myVisualizer.DrawTangents(20, 20, 2.5);
		break;

		case 2:
			myVisualizer.IntegrateAndDraw();
		break;

		case 3:
			myVisualizer.LIC(8);
		break;

		case 4:
			myVisualizer.SpotNoise(125000, 10);
		break;
		
		case 5:
			myVisualizer.TextureAdvection(7, 127);
		break;
	}

	switch (scale)
	{
		case 1:
			myVisualizer.ColorizeMagnitude(rainbow);
		break;

		case 2:
			myVisualizer.ColorizeMagnitude(heat);
		break;

		case 3:
			myVisualizer.ColorizeMagnitude(temp);
		break;

		case 4:
			myVisualizer.ColorizeMagnitude(bluyel);
		break;
	}

	if (critical)
		myVisualizer.FindCriticalPoints(25);

	blit(buffer, screen, 0, 0, 0, 0, 512, 512);

	while(!keypressed() && !mouse_b);

	allegro_exit();
	return 0;
}
END_OF_MAIN();

