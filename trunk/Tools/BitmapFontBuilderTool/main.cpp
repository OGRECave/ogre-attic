
/** Tool designed to take the binary width files from BitmapFontBuilder
    http://www.lmnopc.com/bitmapfontbuilder/ and convert them into 
    Ogre .fontdef 'glyph' statements.
    Highly inelegant, but it works!
*/

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
using namespace std;

int main(int argc, char** argv)
{
    int size;
    std::string datName, newName;

    cout << "Enter size of (square) texture: ";
    cin >> size;
    cout << "Enter name of file containing binary widths: ";
    cin >> datName;
    cout << "Enter name of new text file to create: ";
    cin >> newName;

    int charSize = size / 16;
    int halfWidth = charSize / 2;
    FILE *fp = fopen(datName.c_str(), "rb");

    ofstream o(newName.c_str());

    int posx = 0;
    int posy = size; // work backwards so same as uv
    int colcount = 0;
    for (int c = 0; c < 256; c++, colcount++)
    {
        if (colcount == 16)
        {
            colcount = 0;
            posx = 0;
            posy -= charSize;
        }

        int width = fgetc(fp);
        float thisx_start = posx + halfWidth - (width / 2);
        float thisx_end = posx + halfWidth + (width / 2);

        float u1, u2, v1, v2;
        u1 = thisx_start / (float)(size) ;
        u2 = thisx_end / (float)(size);
        v1 = (float)posy / (float)(size);
        v2 = (float)(posy - charSize) / (float)(size);

        std::string s = " ";
        s.at(0) = c;
        o << "glyph " << s << " " << u1 << " " << v1 << " " << u2 << " " << v2 << std::endl;

        posx += charSize;


    }

    fclose(fp);
    



    return 0;
}