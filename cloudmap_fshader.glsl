#version 330

in vec2 uv;

out vec3 height;

uniform int p[512];
uniform int permutation[256];

int grid_length = 512;

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float t, float a, float b) { return a + t * (b - a); }
float grad(int hash, float x, float y) { 
    return ((hash & 1) == 0 ? x : -x) + ((hash & 2) == 0 ? y : -y);
}

float noise(float x, float y){
    int X = int(x) & 255;
    int Y = int(y) & 255;
    x -= int(x);
    y -= int(y);
    float u = fade(x);
    float v = fade(y);
    int A = (p[X  ] + Y) & 255;
    int B = (p[X+1] + Y) & 255;
    return lerp(v, lerp(u, grad(p[A  ], x, y  ), grad(p[B  ], x-1, y  )),
                   lerp(u, grad(p[A+1], x, y-1), grad(p[B+1], x-1, y-1)));
}

float fBm(vec2 point, float initFreq, float lacunarity, float gain, int octaves){
    float tot=0.0f;
    float freq= initFreq;
    float amplitude=gain;

    for(int i=0; i<octaves; ++i){

        tot+= abs( noise(point[0]*freq, point[1]*freq)-0.2 )*amplitude;
        //update values after adding to total
        freq*=lacunarity;
        amplitude*=gain;
    }
    return tot;
}

void main(){
    for (int i=2; i<10; i+=2) {
        height += vec3(fBm(uv, i, 0.3, 1.5/pow(float(i),1.3), 10));
    }
    height = (height - 0.5)*0.3;
    //height = vec3(fBm(uv, 50, 0.3, 0.1, 10)) + vec3(fBm(uv, 10, 0.2, 0.3, 10)) + 2 * vec3(fBm(uv, 4, 0.2, 0.4, 10)) - 0.5;
    //height = vec3(max(0.0f, min(1.0f, noise(uv[0],uv[1]))));
    //height = vec3(noise(uv.x, uv.y));
}
    
