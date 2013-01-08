#include <PS2Keyboard.h>
#include <TVout.h>
#include <font4x6.h>
#include <SD.h>
#include <bitlash.h>

TVout TV;
PS2Keyboard keyboard;
boolean runf = false;
File f;

numvar pixel(void) {
        numvar ret = TV.get_pixel(getarg(1), getarg(2));
        
        if (getarg(0) > 2)
                TV.set_pixel(getarg(1), getarg(2), getarg(3));
        
        return ret;
}

numvar tonef(void) {
        if (getarg(0) == 2)
                TV.tone(getarg(1), getarg(2));
        else if (getarg(0) == 1)
                TV.tone(getarg(1));
        else
                TV.noTone();        
}

numvar line(void) {
        TV.draw_line(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
}

numvar rect(void) {
        if (getarg(0) == 5)
                TV.draw_rect(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
        else 
                TV.draw_rect(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5), getarg(6));
}

numvar circ(void) {
        if (getarg(0) == 4)
                TV.draw_circle(getarg(1), getarg(2), getarg(3), getarg(4));
        else 
                TV.draw_circle(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
}

numvar cls(void) {
        TV.clear_screen();
        TV.set_cursor(0, 0);
}

numvar readchar(void) {
        long t = 0;
        if (getarg(0) == 1)
                t = millis() + getarg(1);
                
        while ((!t) || (millis() < t)) {
                if (keyboard.available())
                        return keyboard.read();
                if (Serial.available())
                        return Serial.read();
        }
        
        return -1;
}

int readline(char *s, int len) {
        int ret = 0;
        char c;
        
        len--;
        while(ret < len) {
                c = readchar();
                
                if ((c == 13) || (c == 10) || (c == 4))
                          break;
                          
                if (((c == 8) || (c == 0x7F)) && (ret > 0)) {
                          ret--;
                          sendit(8); sendit(' '); sendit(8);
                          continue;
                }
                          
                sendit(c);
                s[ret] = c;
      
                ret++;          
        }
        
        s[ret] = '\0';
        
        sendit('\r');
        sendit('\n');
        
        return ret;
}

numvar sdl(void) {
        SD.root.ls();
}

numvar sdr(void) {
        char name[13];
        
        sendstr("file to read: ");
        readline(name, 13);
        
        f = SD.open(name);
        
        while (f.available())
                sendit(f.read());
                
        f.close();
}

numvar sdw(void) {
        char name[13];
        char c;
        
        sendstr("file to write: ");
        readline(name, 13);
        
        f = SD.open(name, FILE_WRITE);
        
        while (1) {
                c = readchar();
                
                if ((c == 3) || (c == 4))
                        break;
                
                if ((c == 13) || (c == 10)) {
                        sendit('\r');
                        sendit('\n');
                        f.println();

                        continue;
                }
                
                sendit(c);        
                f.print(c);
        }
                
        f.close();
}

numvar sdd(void) {
        char name[13];
        
        sendstr("file to delete: ");
        readline(name, 13);
        
        SD.remove(name);
}

numvar sdx(void) {
        char name[13];
        
        sendstr("file to run: ");
        readline(name, 13);
        
        f = SD.open(name);
        runf = true;
}


void sendstr(char *str) {
        while (*str != '\0') {
                sendit(*str);
                str++;
        }
}

void sendit(byte c) {
        Serial.print(c, BYTE);
        TV.print(c, BYTE);
}

void setup(void) {
        TV.begin(NTSC,128,192);
        TV.select_font(font4x6);
        
        setOutputHandler(&sendit);

	initBitlash(9600);

        addBitlashFunction("pixel", (bitlash_function) pixel);
        addBitlashFunction("tone", (bitlash_function) tonef);
        addBitlashFunction("line", (bitlash_function) line);
        addBitlashFunction("rect", (bitlash_function) rect);
        addBitlashFunction("circ", (bitlash_function) circ);
        addBitlashFunction("clear", (bitlash_function) cls);
        addBitlashFunction("sdl", (bitlash_function) sdl);
        addBitlashFunction("sdr", (bitlash_function) sdr);
        addBitlashFunction("sdw", (bitlash_function) sdw);
        addBitlashFunction("sdd", (bitlash_function) sdd);
        addBitlashFunction("sdx", (bitlash_function) sdx);
        addBitlashFunction("getc", (bitlash_function) readchar);
        
        if (!SD.begin())
                sendstr("SD.begin() failed\n");

        keyboard.begin(4);
}

void loop(void) {
        if (runf) {
                while (f.available())
                        doCharacter(f.read());

                f.close();

                runf = false;
        }
  
        if (keyboard.available())
                doCharacter(keyboard.read());

	runBitlash();
}
