#version 430
# define M_PI 3.14159265358979323846

// Global variables for lighting calculations
//layout(location = 1) uniform vec3 viewPos;
layout (location = 2) uniform sampler2D phasorField;
layout (location = 10) uniform vec3 iResolution;
layout (location = 11) uniform vec4 iMouse;
layout (location = 12) uniform float _f;
layout (location = 13) uniform float _b;
layout (location = 14) uniform int _impPerKernel;

// Output for on-screen color
layout(location = 0) out vec4 fragColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

vec2 fragCoord = fragPos.xy;

//phasor noise parameters
//float _f = 10.0;
//float _b = 2.0;
//float _o = 8.0;
float _kr;
//int _impPerKernel = 16;
int _seed = 6;

vec2 uv;

float gaussian(vec2 x, float b)
{
    float a = exp(-M_PI * (b * b) * ((x.x * x.x) + (x.y * x.y)));
    return a;
}

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



void init_noise()
{
    _kr = sqrt(-log(0.05) / M_PI) / _b;
}


vec2 cell(ivec2 ij, vec2 uv, float b)
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
		float omega = uni(-2.4,2.4);
        vec2 r = vec2(cos(omega),sin(omega));
		noise += gaussian(d, b)*r;
		impulse++;
	}
	return noise;
}

vec2 eval_noise(vec2 uv, float b)
{   
	float cellsz = 2.0 *_kr;
	vec2 _ij = uv / cellsz;
	ivec2  ij = ivec2(_ij);
	vec2  fij = _ij - vec2(ij);
	vec2 noise = vec2(0.0);
	for (int j = -2; j <= 2; j++) {
		for (int i = -2; i <= 2; i++) {
			ivec2 nij = ivec2(i, j);
			noise += cell(ij + nij , fij - vec2(nij),b );
		}
	}
    return noise;
}

void main()
{
  uv = fragCoord/iResolution.y;
  uv.y=-uv.y;
  init_noise();
  float o = iMouse.x/iResolution.x * 2.0*M_PI;
  vec2 gaussian_field = vec2(eval_noise(uv,_b));
  gaussian_field = normalize(gaussian_field);
  float angle = atan(gaussian_field.y,gaussian_field.x)/2.0/M_PI;
  fragColor vec4(vec3(angle,0, 0),1.0);
}