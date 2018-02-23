#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef struct color Color;

struct image {
    int sx;
    int sy;
    double** px;
};

typedef struct image Image;

Image* make_image(int sx, int sy) {
    Image* img = calloc(1,sizeof(Image));
    img->sx = sx;
    img->sy = sy;
    img->px = calloc(sy,sizeof(double*));
    for (int y=0; y<sy; y++) {
        img->px[y] = calloc(sx,sizeof(double));
    }
    return img;
}

void free_image(Image* img) {
    for (int y=0; y<img->sy; y++) {
        free(img->px[y]);
    }
    free(img->px);
    free(img);
}

void save_image(char* filename, Image* img, Color (*colormap)(double v)) {
    FILE* fp = fopen(filename,"wb");
    if (!fp) {
        perror(filename);
        exit(1);
    }
    fprintf(fp,"P6\n");
    fprintf(fp,"%d %d\n",img->sx,img->sy);
    fprintf(fp,"255\n");
    for (int y=0; y<img->sy; y++) {
        for (int x=0; x<img->sx; x++) {
            Color c = colormap(img->px[y][x]);
            fwrite(&c,3,1,fp);
        }
    }
    fclose(fp);
}

struct coord {
    int x;
    int y;
};
typedef struct coord Coord;

Coord bit_unzip(int v) {
    Coord r = {0,0};
    int p=0;
    while (v) {
        int bit = v&1;
        v >>= 1;
        r.x |= bit<<p;
        
        bit = v&1;
        v >>= 1;
        r.y |= bit<<p;
        
        p++;
    }
    return r;
}
    
int bit_zip(Coord c) {
    int v=0;
    int p=0;
    while (c.x || c.y) {
        int bit = c.x & 1;
        c.x >>= 1;
        v |= bit<<p;
        p++;
        
        bit = c.y & 1;
        c.y >>= 1;
        v |= bit<<p;
        p++;
    }
    return v;
}

struct coord3 {
    int x;
    int y;
    int z;
};
typedef struct coord3 Coord3;

Coord3 bit_unzip3(int v) {
    Coord3 r = {0,0,0};
    int p=0;
    while (v) {
        int bit = v&1;
        v >>= 1;
        r.x |= bit<<p;
        
        bit = v&1;
        v >>= 1;
        r.y |= bit<<p;
        
        bit = v&1;
        v >>= 1;
        r.z |= bit<<p;
        
        p++;
    }
    return r;
}
    
int bit_zip3(Coord3 c) {
    int v=0;
    int p=0;
    while (c.x || c.y || c.z) {
        int bit = c.x & 1;
        c.x >>= 1;
        v |= bit<<p;
        p++;
        
        bit = c.y & 1;
        c.y >>= 1;
        v |= bit<<p;
        p++;

        bit = c.z & 1;
        c.z >>= 1;
        v |= bit<<p;
        p++;
    }
    return v;
}

char* bstr(int v) {
    static char buf[64];
    char* p = buf+63;
    *p = '\0';
    while (v) {
        p--;
        *p = (v&1) + '0';
        v >>= 1;
    }
    return p;
}

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

// hastily adapted from: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
Color hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    Color         out;
    double r,g,b;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        r = in.v;
        g = in.v;
        b = in.v;
        
        out.r = 255*r;
        out.g = 255*g;
        out.b = 255*b;        
        return out;
    }
    hh = in.h;
    hh *= 360;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        r = in.v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = in.v;
        b = p;
        break;
    case 2:
        r = p;
        g = in.v;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = in.v;
        break;
    case 4:
        r = t;
        g = p;
        b = in.v;
        break;
    case 5:
    default:
        r = in.v;
        g = p;
        b = q;
        break;
    }
    out.r = 255*r;
    out.g = 255*g;
    out.b = 255*b;
    
    return out;
}

Color map_grey(double v) {
    Color c = {v*255,v*255,v*255};
    return c;
}

Color map_hsv(double v) {
    hsv c = {v,1,1};
    return hsv2rgb(c);
}

Color map_rgb_packed(double v) { 
    int vi = (int)v;
    Color c;
    c.b = vi&0xFF;
    vi >>= 8;
    c.g = vi&0xFF;
    vi >>= 8;
    c.r = vi&0xFF;
    return c;
}

Color map_hsv_packed(double v) { 
    int vi = (int)v;
    
    Color c1;
    c1.b = vi&0xFF;
    vi >>= 8;
    c1.g = vi&0xFF;
    vi >>= 8;
    c1.r = vi&0xFF;
    
    hsv c;
    c.h = (double)c1.r/255.0;
    vi >>= 8;
    c.s = (double)c1.g/255.0;
    vi >>= 8;
    c.v = (double)c1.b/255.0;
    
    Color cc = hsv2rgb(c);
    //printf("c1=%d,%d,%d  hsv={%.3f %.3f %.3f} cc=%d,%d,%d  %c\n",c1.r,c1.g,c1.b, c.h,c.s,c.v, cc.r, cc.g, cc.b , (cc.r==3 && cc.g==0 && cc.b==0) ? 'z' : ' ');
    return cc;
}

// hilbert curve: rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int rx, int ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }

        //Swap x and y
        int t  = *x;
        *x = *y;
        *y = t;
    }
}

// hilbert curve: convert (x,y) to d
int xy2d (int n, int x, int y) {
    int rx, ry, s, d=0;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(s, &x, &y, rx, ry);
    }
    return d;
}

// hilbert curve: convert d to (x,y)
Coord d2xy(int n, int d) {
    Coord r;
    int rx, ry, s, t=d;
    r.x = r.y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, &r.x, &r.x, rx, ry);
        r.x += s * rx;
        r.y += s * ry;
        t /= 4;
    }
    return r;
}

double sawtooth(double v, double multiplier) {
    return fmod(v,(1/multiplier))*multiplier;
    
}

int main(int argc, char* argv[]) {
    /*for (int i=0; i<258; i++) {
        Coord c = bit_unzip(i);
        printf("%3x   %3x %3x   %3x\n",i,c.x,c.y,bit_zip(c));
    }*/
    
    /*
    hsv h = {0.000,0,0};
    Color c = hsv2rgb(h);
    printf("h %.3f %.3f %.3f   c %d %d %d\n",h.h,h.s,h.v,c.r,c.g,c.b);
    */
    
    /*
    // WALK A LINE THROUGH Z CURVE SPACE
    int size = 64;
    if (argc>=2) size=atoi(argv[1]);
    int pixels = size*size;
    printf("Size=%d\n",size);
    Image* img = make_image(size,size);
    double times = 0;
    for (int y=0; y<img->sy; y++) {
        for (int x=0; x<img->sx; x++) {
            //img->px[y][x] = (float)(y*img->sx+x) / (img->sx*img->sy);
            Coord cd = {x,y};
            double v = (double)bit_zip(cd)/pixels;
            if (times) {
                v = sin(v*3.14159*2 * times)*0.5+0.5;
            }
            img->px[y][x] = v;
        }
    }
    char filename[] = "x1.ppm";
    printf("Saving %s...\n",filename);
    save_image(filename,img,map_grey);
    */
    
    /*
    // CONVERT XY COORD TO Z CURVE SPACE, THEN TO RGB OR HSV SPACE
    int size = 4096;
    int pixels = size*size;
    printf("Size=%d\n",size);
    Image* img = make_image(size,size);
    double times = 2;
    for (int y=0; y<img->sy; y++) {
        for (int x=0; x<img->sx; x++) {
            //img->px[y][x] = (float)(y*img->sx+x) / (img->sx*img->sy);
            Coord cd = {x,y};
            double v = (double)bit_zip(cd)/pixels;
            if (times) {
                v = sin(v*3.14159*2 * times)*0.5+0.5;
            }
            Coord3 cd3 = bit_unzip3((int)(v*4096*4096));
            double v_rgb = 256*256*cd3.x + 256*cd3.y + cd3.z;
            //printf("cd3 %d,%d,%d // ",cd3.x,cd3.y,cd3.z);
            //Color c_test = map_hsv_packed(v_rgb);
            v = v_rgb;
            img->px[y][x] = v;
        }
    }
    char filename[] = "x1.ppm";
    printf("Saving %s...\n",filename);
    save_image(filename,img,map_hsv_packed);
    */
    
    // HILBERT CURVE WALK
    int h_depth = 6;
    if (argc>=2) h_depth=atoi(argv[1]);
    int size = 1<<h_depth;
    int pixels = size*size;
    printf("H=%d Size=%d\n",h_depth,size);
    Image* img = make_image(size,size);
    double times = 81*81;
    for (int y=0; y<img->sy; y++) {
        for (int x=0; x<img->sx; x++) {
            //img->px[y][x] = (float)(y*img->sx+x) / (img->sx*img->sy);
            Coord cd = {x,y};
            int d = xy2d(size,cd.x,cd.y);
            //printf("%d (%d,%d)\n",d,x,y);
            double v = (double)d/pixels;
            if (times) {
                v = sin(v*3.14159*2 * times)*0.5+0.5;
                //v = sawtooth(v,times);
            }
            img->px[y][x] = v;
        }
    }
    char filename[] = "x1.ppm";
    printf("Saving %s...\n",filename);
    save_image(filename,img,map_hsv);
   
    
        
}