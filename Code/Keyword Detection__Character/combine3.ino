#include "Mic.h"
#include "classifier.h"

#include <Adafruit_GFX.h>   // Core graphics library
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <gamma.h>



// tune as per your needs
#define SAMPLES 64
#define GAIN (1.0f/50)
#define SOUND_THRESHOLD 1000


float features[SAMPLES];
Mic mic;
Eloquent::ML::Port::SVM clf;





uint8_t yes_pattern[8][8] = {
    {2,0,2,0,0,0,0,0},
    {0,2,0,0,0,0,0,0},
    {0,2,0,2,2,2,0,0},
    {0,0,0,2,2,0,0,0},
    {0,0,0,2,2,2,0,0},
    {0,0,0,0,0,2,2,2},
    {0,0,0,0,0,2,2,0},
    {0,0,0,0,2,2,2,0}
};



uint8_t no_pattern[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {1,0,0,1,0,1,1,0},
    {1,1,0,1,1,0,0,1},
    {1,0,1,1,1,0,0,1},
    {1,0,0,1,0,1,1,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};



uint8_t straight[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {6,6,6,6,6,6,6,6},
    {6,6,6,6,6,6,6,6},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

uint8_t bigmouth[8][8] = {
    {0,0,6,6,6,6,0,0},
    {0,6,0,0,0,0,6,0},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {0,6,0,0,0,0,6,0},
    {0,0,6,6,6,6,0,0}
};

uint8_t mediummouth[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,6,6,6,6,0,0},
    {0,6,0,0,0,0,6,0},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {0,6,0,0,0,0,6,0},
    {0,0,6,6,6,6,0,0},
    {0,0,0,0,0,0,0,0}
};

uint8_t smallmouth[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,6,6,6,6,6,6,0},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {0,6,6,6,6,6,6,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

uint8_t extrasmallmouth[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,6,6,6,6,6,6,0},
    {6,0,0,0,0,0,0,6},
    {6,0,0,0,0,0,0,6},
    {0,6,6,6,6,6,6,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

uint8_t happy[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {3,0,0,0,0,0,0,3},
    {3,3,0,0,0,0,3,3},
    {0,3,3,3,3,3,3,0},
    {0,0,3,3,3,3,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};


uint8_t sadface[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,0},
    {1,1,0,0,0,0,1,1},
    {1,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};



int v_pattern = 0;

String keyword = "None";
uint16_t palette[8] = {};



// Width, Height, Pinnumber
// + First Pixel Start at the top ( NEO_MATRIX_BOTTOM )
// + First Pixel Start at the Right ( EO_MATRIX_LEFT )
// + NEO_MATRIX_COLUMNS or NEO_MATRIX_ROWS 
//  ( Row major means the pixels are arranged in horizontal lines (the opposite, in vertical lines, is column major). )
// + NEO_MATRIX_PROGRESSIVE or NEO_MATRIX_ZIGZAG
//  ( Progressive means each row proceeds in the same direction.  )
// + Type of led

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 6,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);


void setup() {
    matrix.begin();
    palette[0] = matrix.Color(0,0,0);
    palette[1] = matrix.Color(255,0,0);
    palette[2] = matrix.Color(0,255,0);
    palette[3] = matrix.Color(0,0,255);
    palette[4] = matrix.Color(0,255,255);
    palette[5] = matrix.Color(255,0,255);
    palette[6] = matrix.Color(255,255,0);
    palette[7] = matrix.Color(255,255,255);
  
    
    
    Serial.begin(115200);
    PDM.onReceive(onAudio);
    mic.begin();
    delay(3000);

}


void loop() {
    
    s();
     
    
    if (recordAudioSample()) {
        Serial.print("You said: ");
        Serial.println(clf.predictLabel(features));
        keyword = clf.predictLabel(features);
    
        if (keyword == "no")
        {
          no();
        }
        else if (keyword == "yes" )
        {
          yes();
        } 
        

        delay(1000);
    }
    //keyword = "None";
    delay(20);
}


/**
 * PDM callback to update mic object
 */
void onAudio() {
    mic.update();
}


/**
 * Read given number of samples from mic
 */

bool recordAudioSample() {
    if (mic.hasData() && mic.data() > SOUND_THRESHOLD) {

        for (int i = 0; i < SAMPLES; i++) {
            while (!mic.hasData())
                delay(1);

            features[i] = mic.pop() * GAIN;
        }

        return true;
    }

    return false;
}

int volumedetection()
{
  if (mic.hasData() && mic.data() > SOUND_THRESHOLD) 
  {
    if ( mic.data() > 100 &&  mic.data() < 300 )
    {
      v_pattern = 1;
    }
    else if ( mic.data() > 300 )
    {
      v_pattern = 2;
    }
  }
  else  
  {
    v_pattern = 0;
  }
  
}

void s(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = straight[x][y];
           
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(100);
}


void m1(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = extrasmallmouth[x][y];
           
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(200);
}


void m2(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = smallmouth[x][y];
            
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(100);
}

void m3(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = mediummouth[x][y];
          
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(100);
}

void m4(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = bigmouth[x][y];
           
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(100);
}

void h(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = happy[x][y];
            
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(1000);
}

void sa(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = sadface[x][y];
            
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(1000);
}

void no(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = no_pattern[x][y];
            
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(2000);

}

void yes(){
    for(int x = 0; x<8; x++){
        for(int y = 0; y<8; y++){
            uint8_t index = yes_pattern[x][y];
            
            matrix.drawPixel(x, y, palette[index]); 
        }
    }

    matrix.show();
    delay(2000);
}
