#version 430
# define M_PI 3.14159265358979323846

// Global variables for lighting calculations
//layout(location = 1) uniform vec3 viewPos;
layout (location = 2) uniform sampler2D phaseField;
layout (location = 3) uniform sampler2D dogImage;
layout (location = 12) uniform float _f;
layout (location = 13) uniform float _b;
layout (location = 14) uniform int _impPerKernel;
layout (location = 31) uniform bool first;
layout (location = 32) uniform bool second;
layout (location = 33) uniform bool third;
layout (location = 34) uniform bool fourth;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

vec2 fragCoord = fragPos.xy;

//phasor noise parameters
//float _f = 40.0;
//float _b = 30.0;
//float _o = 8.0;
float _kr;
//int _impPerKernel = 16;
int _seed = 1;

vec2 uv;


///////////////////////////////////////////////
//prng
///////////////////////////////////////////////

int N = 15487469;
int x_;
void seed(int s){x_ = s;}
int next() { x_ *= 3039177861; x_ = x_ % N;return x_; }
float uni_0_1() {return  float(next()) / float(N);}
float uni(float min, float max){ return min + (uni_0_1() * (max - min));}


int morton(int x, int y)
{
  int z = 0;
  for (int i = 0 ; i < 32* 4 ; i++) {
    z |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
  }
  return z;
}



vec2 phasor(vec2 x, float f, float b, float o, float phi)
{
    
    float a = exp(-M_PI * (b * b) * ((x.x * x.x) + (x.y * x.y)));
    float s = sin (2.0* M_PI * f  * (x.x*cos(o) + x.y*sin(o))+phi);
    float c = cos (2.0* M_PI * f  * (x.x*cos(o) + x.y*sin(o))+phi);
    return vec2(a*c,a*s);
}



void init_noise()
{
    _kr = sqrt(-log(0.05) / M_PI) / _b;
}


vec2 cell(ivec2 ij, vec2 uv, float f, float b)
{
	int s= morton(ij.x,ij.y) + 333;
	s = s==0? 1: s +_seed;
	seed(s);
	int impulse  =0;
	int nImpulse = _impPerKernel;
	float  cellsz = 2.0 * _kr;
	vec2 noise = vec2(0.0);
	while (impulse <= nImpulse){
		vec2 impulse_centre = vec2(uni_0_1(),uni_0_1());
		vec2 d = (uv - impulse_centre) *cellsz;
		float rp = uni(0.0,2.0*M_PI) ;
        vec2 trueUv = ((vec2(ij) + impulse_centre) *cellsz);
		trueUv.y = -trueUv.y;
        float o = texture(phaseField, trueUv).x *2.0* M_PI;
		noise += phasor(d, f, b ,o, rp);
		impulse++;
	}
	return noise;
}

vec2 eval_noise(vec2 uv, float f, float b)
{   
	float cellsz = 2.0 *_kr;
	vec2 _ij = uv / cellsz;
	ivec2  ij = ivec2(_ij);
	vec2  fij = _ij - vec2(ij);
	vec2 noise = vec2(0.0);
	for (int j = -2; j <= 2; j++) {
		for (int i = -2; i <= 2; i++) {
			ivec2 nij = ivec2(i, j);
			noise += cell(ij + nij , fij - vec2(nij),f,b);
		}
	}
    return noise;
}

float PWM(float x, float r)
{
	return mod(x,2.0*M_PI)> 2.0*M_PI *r ? 1.0 : 0.0; 
}

float square(float x)
{
  return PWM(x,0.5);   
}

float sawTooth(float x)
{
	return mod(x,2.0*M_PI)/(2.0*M_PI);
}

void main()
{
    uv = fragCoord;
    uv.y=-uv.y;
    uv.x = abs(uv.x);
    init_noise();
    float o = uv.x * 2.0*M_PI;
    vec2 phasorNoise = eval_noise(uv,_f,_b);
    vec2 dir = vec2(cos(o),sin(o));
    float phi = atan(phasorNoise.y,phasorNoise.x);
    float I = length(phasorNoise);
    float angle = texture(phaseField, vec2(1.0-abs(fragCoord.x), fragCoord.y) ).x;
    
    float p1 = 0.0;
    float g1 = 0.0;
    if (first){
        p1 = PWM(phi, uv.x+0.2 *0.5);
        g1 = exp(-(uv.x-0.2)*(uv.x-0.2)*20.0);
    }

    float p2 = 0.0;
    float g2 = 0.0;
    if (second){
        p2 = sawTooth(phi);
        g2 = exp(-(uv.x-0.4)*(uv.x-0.4)*20.0);
    }
    float p3 = 0.0;
    float g3 = 0.0;

    if (third){
        p3 = sin(phi+M_PI)+0.5*0.5;
        g3 = exp(-(uv.x-0.8)*(uv.x-0.8)*20.0);
    }
    float p4 = 0.0;
    float g4 = 0.0;
    if (fourth){
        p4 = sawTooth(phi+M_PI/2);
        g4 = exp(-(uv.x-0.1)*(uv.x-0.1)*20.0);
    }
    
    vec3 phasorfield  = vec3(sin(phi)*0.3 +0.5);
    
    float profile =p1*g1+p2*g2+p3*g3+p4*g4;
    float sumGaus= g1+g2+g3+g4;
    
    phasorfield = vec3(profile/sumGaus);
    outColor = vec4(phasorfield,1.0);
}
