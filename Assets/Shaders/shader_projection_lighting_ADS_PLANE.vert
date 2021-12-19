#version 440 core
layout (location = 0) in vec3 Position; //vertex positions
layout (location=1) in vec2 texCoord;	//tex coords
layout (location=2) in vec3 normal;	// vertex normals

 						
out vec2 textureCoordinate;
out vec3 normals;
out vec3 fragmentPosition;
out vec3 lightColour;
out vec3 lightPosition;
out vec3 viewPosition;
out float time;
out vec3 position;

uniform mat4 uNormalMatrix;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uLightColour;
uniform vec3 uAmbientIntensity;
uniform vec3 lightCol;
uniform vec3 uLightPosition;
uniform vec3 uViewPosition;
uniform float uTime; 

// in: vec2 out:random float value
//'fract()' returns only the fractional part of a value.
//Deterministic - not really random
//Introduce time to get progression
float random (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123  );
}

//Value noise function
//in: vec2 out: float value
float noiseFunction(vec2 st) {
    vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(st);
    vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(st));
    
return mix(mix(random(b), random(b + d.yx), f.x), mix(random(b + d.xy), random(b + d.yy), f.x), f.y);
}

//  Classic Perlin 2D Noise  
//  by Stefan Gustavson 
// 
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);} 
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;} 
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); } 
 
float cnoise(vec2 P){ 
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0); 
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0); 
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation 
  vec4 ix = Pi.xzxz; 
  vec4 iy = Pi.yyww; 
  vec4 fx = Pf.xzxz; 
  vec4 fy = Pf.yyww; 
  vec4 i = permute(permute(ix) + iy); 
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024... 
  vec4 gy = abs(gx) - 0.5; 
  vec4 tx = floor(gx + 0.5); 
  gx = gx - tx; 
  vec2 g00 = vec2(gx.x,gy.x); 
  vec2 g10 = vec2(gx.y,gy.y); 
  vec2 g01 = vec2(gx.z,gy.z); 
  vec2 g11 = vec2(gx.w,gy.w); 
  vec4 norm = 1.79284291400159 - 0.85373472095314 *  
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)); 
  g00 *= norm.x; 
  g01 *= norm.y; 
  g10 *= norm.z; 
  g11 *= norm.w; 
  float n00 = dot(g00, vec2(fx.x, fy.x)); 
  float n10 = dot(g10, vec2(fx.y, fy.y)); 
  float n01 = dot(g01, vec2(fx.z, fy.z)); 
  float n11 = dot(g11, vec2(fx.w, fy.w)); 
  vec2 fade_xy = fade(Pf.xy); 
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x); 
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y); 
  return 2.3 * n_xy; 
}


void main()
{
	vec2 uv = texCoord;
	uv *= 10.0;
	
	vec3 pos = Position;
	
	float ovn = 1.0*(cnoise(uv ));
    ovn += 1.0*(cnoise(uv   *2.0 )*0.5);
    ovn +=  1.0*(cnoise(uv   *4.0 )*0.25);
    ovn +=  1.0*(cnoise(uv   *8.0 )*0.125);
	ovn +=  1.0*(cnoise(uv   *16.0 )*0.0625);
    ovn +=  1.0*(cnoise(uv   *32.0 )*0.03125);
	
	
	//Alter the positions of the y coordinate of the vertices
	//Use the noise function
	pos.y = (pos.y+(ovn/4.0))-(1.0-(sin(ovn))*1.6);
	// Noise + time (waves)
	//pos.y = (pos.y+(ovn/3.0))-(1.0-(sin(ovn*(uTime/100)))*0.5);
	
	

	gl_Position = uProjection * uView * uModel * vec4(pos.x, pos.y, pos.z, 1.0); 
						
	textureCoordinate = vec2(texCoord.x, 1 - texCoord.y);
	
	//get the fragment position in world coordinates as this is where the lighting will be calculated
	fragmentPosition = vec3(uModel * vec4(Position, 1.0f));
	//fragmentPosition = vec3(uModel * vec4(pos, 1.0f));
	
	
	//pass the normals to the fragment shader unmodified
	//normals = normal;
	
	//pass normals to fragment shader after modifying for scaling
	//calculate a 'normal matrix' and multiply by the unmodified normal
	normals = mat3(uNormalMatrix) * normal;
	
	
	lightColour = uLightColour;
	lightPosition = uLightPosition;
	viewPosition = uViewPosition;
	time = uTime;
	//Send the vertext position data to the fragment shader
	position = pos;

}