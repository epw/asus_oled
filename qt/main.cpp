/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@users.berlios.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include <qimage.h>
#include <qfile.h>

#include <stdlib.h>
#include <string.h>

#include <iostream>

using namespace std;

#define VERSION			"0.03"

#define ACTION_HELP		0
#define ACTION_DISABLE		1
#define ACTION_ENABLE		2
#define ACTION_STATIC		3
#define ACTION_ROLL		4
#define ACTION_FLASH		5
#define ACTION_FLASH_ROLL	6

#define OUTPUT_TYPE_BIN		0
#define OUTPUT_TYPE_ASCII	1

#define MAX_WIDTH		1792
#define MAX_HEIGHT		32
#define DATA_SIZE		(MAX_WIDTH*MAX_HEIGHT)

#define COL_BLACK		0
#define COL_WHITE		1
#define COL_BLACK_CORR		2

/* Data buf used for our image map */
uint8_t data[DATA_SIZE];

#define DATA(x,y)		(data[(y)*MAX_WIDTH + (x)])

void show_help()
{
	cout << "\nA program converting graphic files to the format\n";
	cout << "recognized by Asus OLED kernel module.\n\n";

	cout << "Usage: asus_oled [options]\n\n";
	cout << "Possible options (more than one option is accepted at a time):\n";
	cout << " -e       - enables the display (may be used together with any other option like -s -r ...)\n";
	cout << " -d       - disables the display (may be used together with any other option like -s -r ...)\n";
	cout << " -s  file - creates static image from given graphic file\n";
	cout << " -r  file - creates rolling image from given graphic file\n";
	cout << " -f  file - creates flashing image from given graphic file\n";
	cout << " -fr file - creates flashing and rolling image from given graphic file\n";
	cout << " -h  num  - sets desired height of the picture to 'num'. Possible ranges:\n";
	cout << "             <1; 32> in 's' and 'r' modes\n";
	cout << "             <1; 16> in 'f' mode\n";
	cout << "             <17; 32> in 'fr' mode\n";
	cout << "             This parameter is optional\n";
	cout << " -w  num  - sets desired width of the picture to 'num'. Possible ranges:\n";
	cout << "             <1; 128> in 's', 'f' and 'fr' modes\n";
	cout << "             <1; 1792> in 'r' mode\n";
	cout << "             This parameter is optional\n";
	cout << " -o  file - writes output data to 'file'. If output file is not specified, the\n";
	cout << "             data will be written to /sys/class/asus_oled/... file  (if found)\n";
	cout << " -i  num  - sets the OLED device number. If multiple devices are present, file\n";
	cout << "             controlling them will be in /sys/class/asus_oled/oled_N/ directory,\n";
	cout << "             where N is the device number. The default value is 1 (first device).\n";
	cout << "\n Modificators:\n";
	cout << " -a       - sets output data format to ASCII (# and ' ' - space)\n";
	cout << " -0       - sets output data format to BIN (1 and 0)\n";
	cout << "\n Image manipulation:\n";
	cout << " -inv     - invert the values\n";
	cout << "\n * BlackBackground operation - it sets background pixels to 'black', leaving only 1 white pixel\n";
	cout << "     from one side between now-black background and the black target in center\n";
	cout << " -bb_l    - BlackBackground operation from left side of the picture\n";
	cout << " -bb_r    - BlackBackground operation from right side of the picture\n";
	cout << " -bb_t    - BlackBackground operation from top side of the picture\n";
	cout << " -bb_b    - BlackBackground operation from bottom side of the picture\n";
	cout << " -bb_h    - Alias for bb_l and bb_r - horizontal\n";
	cout << " -bb_v    - Alias for bb_t and bb_b - vertical\n";
	cout << " -bb      - Alias for all bb operations\n";
	cout << "\n       Any set of -bb_* parameters might be used at a time\n";
	cout << "\n";
	cout << " -V       - shows version and quits\n";
	cout << " -?       - shows this help and quits\n";

	cout << "\nVisit http://lapsus.berlios.de/asus_oled.html for details.\n\n\n";
	cout << "Copyright (c) 2007 Jakub Schmidtke\n\n";
	cout << "This program is distributed under the terms of the GPL v2.\n";
	cout << "Visit http://www.gnu.org/licenses/gpl.html for details.\n\n";
}

void show_version()
{
	cout << "Asus OLED data converter and control utility ver. " VERSION "\n";
	return;
}


uint8_t get_filename(int argc, char *argv[], int i, char **file)
{
	if (i >= argc || strlen(argv[i]) < 1)
	{
		cerr << "Missing filename for option '" << argv[i - 1] << "'!\n"
			"Try running the program with '--help' option to see all parameters.\n";
		return 0;
	}

	*file = argv[i];

	return 1;
}

uint8_t get_int(int argc, char *argv[], int i, uint32_t *val)
{
	if (i >= argc || strlen(argv[i]) < 1)
	{
		cerr << "Missing parameter for option '" << argv[i - 1] << "'!\n"
			"Try running the program with '--help' option to see all parameters.\n";
		return 0;
	}

	int v = atoi(argv[i]);

	if (v < 1)
	{
		cerr << "Illegal parameter for option '" << argv[i - 1] << "'!\n"
			"Try running the program with '--help' option to see all parameters.\n";
		return 0;
	}

	*val = (uint32_t) v;

	return 1;
}

uint8_t correct_pixel(int x, int y, int width, int height)
{
	if (x >= width || y >= height) return 0;

	if (DATA(x,y) == COL_BLACK) return 0;

	if (DATA(x,y) == COL_BLACK_CORR) return 1;

	if ((x+1 >= width || DATA(x+1,y) != COL_BLACK)
		&& (x-1 < 0 || DATA(x-1,y) != COL_BLACK)
		&& (y+1 >= height || DATA(x,y+1) != COL_BLACK)
		&& (y-1 < 0 || DATA(x,y-1) != COL_BLACK))
	{

		DATA(x,y) = COL_BLACK_CORR;
		return 1;
	}

	return 0;

}

int set_oled_enabled(uint8_t devnum, bool do_enabl)
{
	QFile file( QString("/sys/class/asus_oled/oled_%1/enabled").arg(devnum) );

	if (!file.exists())
	{
		cerr << "OLED device number " << (int) devnum << " not found!\n";
		return EXIT_FAILURE;
	}

	if ( file.open( IO_WriteOnly ) )
	{
		QTextStream stream( &file );

		if (do_enabl) stream << "1\n";
		else stream << "0\n";

		file.close();
	}
	else
	{
			// TODO error msg
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main( int argc, char *argv[] )
{
	/* What action should be performed? */
	uint8_t what_action = ACTION_HELP;

	/* Should there also be enable/disable action? */
	uint8_t action_enable = ACTION_HELP;

	/* Type of the output to be generated ( zeros and ones for BIN and # and spaces for ASCII) */
	uint8_t output_type = OUTPUT_TYPE_BIN;

	/* Optional device number */
	uint32_t devnum = 1;

	/* Optional width/height specified */
	uint32_t width = 0;
	uint32_t height = 0;

	/* Special operations on data */
	/* Invert values */
	uint8_t do_invert = 0;

	/* Do 'BlackBackground' operation */
	uint8_t do_bb_l = 0;
	uint8_t do_bb_r = 0;
	uint8_t do_bb_t = 0;
	uint8_t do_bb_b = 0;

	/* Input file */
	char *input_file = 0;

	/* File where the output should be written instead of writing to /sys/... file */
	char *output_file = 0;

	/* Should the devnum be automatically found (first existing)? */
	bool autoDevnum = true;
	
	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp("-?", argv[i]) || !strcmp("--help", argv[i]))
		{
			show_help();
			return EXIT_SUCCESS;
		}
		else if (!strcmp("-V", argv[i]))
		{
			show_version();
			return EXIT_SUCCESS;
		}
		else if (!strcmp("-e", argv[i]))
		{
			action_enable = what_action = ACTION_ENABLE;
		}
		else if (!strcmp("-d", argv[i]))
		{
			action_enable = what_action = ACTION_DISABLE;
		}
		else if (!strcmp("-a", argv[i]))
		{
			output_type = OUTPUT_TYPE_ASCII;
		}
		else if (!strcmp("-0", argv[i]))
		{
			output_type = OUTPUT_TYPE_BIN;
		}
		else if (!strcmp("-inv", argv[i]))
		{
			do_invert = 1;
		}
		else if (!strcmp("-bb_l", argv[i]))
		{
			do_bb_l = 1;
		}
		else if (!strcmp("-bb_r", argv[i]))
		{
			do_bb_r = 1;
		}
		else if (!strcmp("-bb_t", argv[i]))
		{
			do_bb_t = 1;
		}
		else if (!strcmp("-bb_b", argv[i]))
		{
			do_bb_b = 1;
		}
		else if (!strcmp("-bb_h", argv[i]))
		{
			do_bb_r = 1;
			do_bb_l = 1;
		}
		else if (!strcmp("-bb_v", argv[i]))
		{
			do_bb_t = 1;
			do_bb_b = 1;
		}
		else if (!strcmp("-bb", argv[i]))
		{
			do_bb_t = 1;
			do_bb_b = 1;
			do_bb_l = 1;
			do_bb_r = 1;
		}
		else if (!strcmp("-s", argv[i]))
		{
			what_action = ACTION_STATIC;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-r", argv[i]))
		{
			what_action = ACTION_ROLL;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-f", argv[i]))
		{
			what_action = ACTION_FLASH;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-fr", argv[i]) || !strcasecmp("-rf", argv[i]))
		{
			what_action = ACTION_FLASH_ROLL;
			i++;
			if (!get_filename(argc, argv, i, &input_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-o", argv[i]))
		{
			i++;
			if (!get_filename(argc, argv, i, &output_file))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-w", argv[i]))
		{
			i++;
			if (!get_int(argc, argv, i, &width))
				return EXIT_FAILURE;
		}
		else if (!strcmp("-i", argv[i]))
		{
			i++;
			if (!get_int(argc, argv, i, &devnum))
				return EXIT_FAILURE;
			
			autoDevnum = false;
		}
		else if (!strcmp("-h", argv[i]))
		{
			i++;
			if (!get_int(argc, argv, i, &height))
				return EXIT_FAILURE;
		}
		else
		{
			cerr << "Incorrect parameter: '" << argv[i] << "'\n";
			show_help();
			return EXIT_FAILURE;
		}
	}

	if (what_action == ACTION_HELP)
	{
		cerr << "No action specified.\n";
		show_help();
		return EXIT_FAILURE;
	}

	if (autoDevnum)
	{
		for (uint32_t i = 1; i < 33; ++i)
		{
			if (QFile( QString("/sys/class/asus_oled/oled_%1/enabled").arg(i)).exists())
			{
				devnum = i;
				break;
			}
		}
	}
	
	cout << "Using OLED device number: " << (int) devnum << "\n";
	
	if (what_action != ACTION_ENABLE && what_action != ACTION_DISABLE)
	{
		if (height > 0)
		{
			if (what_action == ACTION_FLASH && height > 16)
			{
				cerr << "Incorrect height for selected picture mode (>16)\n";
				return EXIT_FAILURE;
			}
			else if (what_action == ACTION_FLASH_ROLL && height < 17)
			{
				cerr << "Incorrect height for selected picture mode (<17)\n";
				return EXIT_FAILURE;
			}
			else if (height > 32)
			{
				cerr << "Incorrect height (> 32)\n";
				return EXIT_FAILURE;
			}
		}

		if (width > 0)
		{
			if (width > 128 && what_action != ACTION_ROLL)
			{
				cerr << "Incorrect width for selected picture mode (>128)\n";
				return EXIT_FAILURE;
			}
			else if (width > 1792)
			{
				cerr << "Incorrect width (>1792)\n";
				return EXIT_FAILURE;
			}
		}

		if (!input_file)
		{
			cerr << "No input file specified!\n";
			return EXIT_FAILURE;
		}

		QImage img = QImage(input_file);

		if (img.isNull())
		{
			cerr << "Opening file '" << input_file << "' failed, or unsupported file format.\n";
			return EXIT_FAILURE;
		}

		uint32_t w, h;

		w = (uint32_t) img.width();
		h = (uint32_t) img.height();

		/* not set */
		if (height < 1)
		{
			/* User didn't specify the height - lets find something reasonable */

			/* But there is width - lets try to calculate height from that */
			if (width > 0)
			{
				height = (h * width) / w;

				if (height > 32) height = 32;

				if (height > 16 && what_action == ACTION_FLASH) height = 16;
			}
			else if (what_action == ACTION_FLASH) height = 16;
			else height = 32;

			if (h < height) height = h;

			if (height < 17 && what_action == ACTION_FLASH_ROLL) height = 32;
		}

		/* not set */
		if (width < 1)
		{
			/* User didn't specify width. We already have desired height,
				so we can calculate width for that height. */
			width = (w * height) / h;

			/* Now check if it is correct for chosen picture mode, and if not,
				correct it. */
			if (width > 1792) width = 1792;

			if (what_action != ACTION_ROLL && width > 128) width = 128;
		}

		/* Lets scale the image */
		QImage scaled(img.smoothScale(width, height, QImage::ScaleFree));

		if (scaled.isNull())
		{
			cerr << "Scaling the image failed!\n";
			return EXIT_FAILURE;
		}

		/* And convert it to B&W */
		img = scaled.convertDepth(1, Qt::MonoOnly | Qt::DiffuseDither);

		if (img.isNull())
		{
			cerr << "Converting image to Black&White failed!\n";
			return EXIT_FAILURE;
		}

		if ((uint32_t) img.width() != width || (uint32_t) img.height() != height)
		{
			cerr << "Converted image has different than expected dimensions: "
					<< (int) img.width() << ", " << (int) img.height() << " vs "
					<< (int) width << ", " << (int) height << "!\n";
			return EXIT_FAILURE;
		}

		memset(data, do_invert?COL_WHITE:COL_BLACK, DATA_SIZE);

		for (uint32_t x = 0; x < width; ++x)
			for (uint32_t y = 0; y < height; ++y)
			{
				if (!img.valid(x, y))
				{
					cerr << "Pixel " << (int) x << ", " << (int) y << " is invalid!\n";
					return EXIT_FAILURE;
				}

				if (qGray(img.pixel(x, y)))
				{
					DATA(x,y) = do_invert?COL_BLACK:COL_WHITE;
				}
			}

		if (do_bb_t || do_bb_b)
		{
			uint32_t x, y;

			for (x = 0; x < width; ++x)
			{
				if (do_bb_t)
					for (y = 0; y < height; ++y)
						if (!correct_pixel(x, y, width, height)) break;

				if (do_bb_b)
					for (y = height-1; y >= 0; --y)
						if (!correct_pixel(x, y, width, height)) break;
			}
		}

		if (do_bb_l || do_bb_r)
		{
			uint32_t x, y;

			for (y = 0; y < height; ++y)
			{
				if (do_bb_l)
					for (x = 0; x < width; ++x)
						if (!correct_pixel(x, y, width, height)) break;

				if (do_bb_r)
					for (x = width-1; x >= 0; --x)
						if (!correct_pixel(x, y, width, height)) break;
			}
		}

		/* Our data is prepared */
	}

	/* If requested, disable the display */
	if (action_enable == ACTION_DISABLE)
	{
		if (set_oled_enabled(devnum, false) == EXIT_FAILURE) return EXIT_FAILURE;
	}

	if (what_action != ACTION_ENABLE && what_action != ACTION_DISABLE)
	{
		QString buf;

		char mode;

		switch(what_action)
		{
			case ACTION_STATIC: mode = 's';
			break;
			case ACTION_ROLL: mode = 'r';
			break;

			/* Difference between flashing and flashing&rolling is only in image size */
			case ACTION_FLASH:
			case ACTION_FLASH_ROLL: mode = 'f';
			break;

			default: mode = 'x';
			break;
		}

		buf.append(QString("<%1:%2x%3>\n").arg(mode).arg(width).arg(height));

		for(uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				if (data[y*MAX_WIDTH + x] == COL_WHITE)
					buf.append((output_type == OUTPUT_TYPE_BIN)?"1":"#");
				else
					buf.append((output_type == OUTPUT_TYPE_BIN)?"0":" ");
			}

			buf.append("\n");
		}

		QString name;

		if (!output_file)
		{
			name = QString("/sys/class/asus_oled/oled_%1/picture").arg(devnum);

			if (!QFile::exists(name))
			{
				cerr << "OLED device number " << (int) devnum << " not found!\n";
				return EXIT_FAILURE;
			}
		}
		else
		{
			name = output_file;
		}

		QFile file(name);

		if ( file.open( IO_WriteOnly ) )
		{
			QTextStream stream( &file );
			stream << buf;
			file.close();
		}
		else
		{
			// TODO error msg
			return EXIT_FAILURE;
		}
	}

	/* If requested, enable the display */
	if (action_enable == ACTION_ENABLE)
	{
		if (set_oled_enabled(devnum, true) == EXIT_FAILURE) return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
