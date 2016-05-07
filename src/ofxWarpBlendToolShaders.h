#pragma once

#define STRINGIFY(A) #A

namespace ofxWarpBlendTool {

static const string VertShaderProgrammable = "#version 150\n" STRINGIFY(
	uniform mat4 modelViewProjectionMatrix;

	in vec4  position;
	in vec2  texcoord;
	in vec4  color;
	in vec3  normal;

	out vec4 colorVarying;
	out vec2 texCoordVarying;
	out vec4 normalVarying;

	void main()
	{
		colorVarying = color;
		texCoordVarying = texcoord; 
		gl_Position = modelViewProjectionMatrix * position;
	}
);

static const string NormalizedFragShaderProgrammable =  "#version 150\n" STRINGIFY(
    out vec4 fragColor;
    
    uniform sampler2D tex0;
    uniform float sat;
    uniform float brt;
    uniform float con;
    
    uniform float rMult;
    uniform float gMult;
    uniform float bMult;
    
    in float depth;
    in vec4 colorVarying;
    in vec2 texCoordVarying;

    vec4 gammaCorrection(vec3 color, vec3 gamma){				   
        return vec4( pow(color.r, gamma.r), pow(color.g, gamma.g), pow(color.b, gamma.b), 1.0 );			}
												   
    vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)
    {
        // Increase or decrease theese values to adjust r, g and b color channels seperately
        const float AvgLumR = 0.5;
        const float AvgLumG = 0.5;
        const float AvgLumB = 0.5;
													   
        const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
													   
        vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
        vec3 brtColor = color * brt;
        vec3 intensity = vec3(dot(brtColor, LumCoeff));
        vec3 satColor = mix(intensity, brtColor, sat);
        vec3 conColor = mix(AvgLumin, satColor, con);
        return conColor;
    }
    
    void main(){
        vec4 texture = texture(tex0, texCoordVarying);
        fragColor = vec4(ContrastSaturationBrightness(texture.rgb,brt,sat,con),texture.a);
        fragColor.rgb = vec3(fragColor.r*rMult, fragColor.g*gMult, fragColor.b*bMult);
    }
);

static const string UnnormalizedFragShaderProgrammable =  "#version 150\n" STRINGIFY(
    out vec4 fragColor;
    
    uniform sampler2DRect tex0;
    uniform float sat;
    uniform float brt;
    uniform float con;
    
    uniform float rMult;
    uniform float gMult;
    uniform float bMult;
    
    in float depth;
    in vec4 colorVarying;
    in vec2 texCoordVarying;

    vec4 gammaCorrection(vec3 color, vec3 gamma){				   
        return vec4( pow(color.r, gamma.r), pow(color.g, gamma.g), pow(color.b, gamma.b), 1.0 );			}
												   
    vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)
    {
        // Increase or decrease theese values to adjust r, g and b color channels seperately
        const float AvgLumR = 0.5;
        const float AvgLumG = 0.5;
        const float AvgLumB = 0.5;
													   
        const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
													   
        vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
        vec3 brtColor = color * brt;
        vec3 intensity = vec3(dot(brtColor, LumCoeff));
        vec3 satColor = mix(intensity, brtColor, sat);
        vec3 conColor = mix(AvgLumin, satColor, con);
        return conColor;
    }
    
    void main(){
        vec4 texture = texture2DRect(tex0, texCoordVarying);
        fragColor = vec4(ContrastSaturationBrightness(texture.rgb,brt,sat,con),texture.a);
        fragColor.rgb = vec3(fragColor.r*rMult, fragColor.g*gMult, fragColor.b*bMult);
    }
);

static string VertShader = STRINGIFY(
    void main(void){
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = ftransform();
											 
        gl_FrontColor = gl_Color;
    }
    );
	
static string NormalizedFragShader = STRINGIFY(
    uniform sampler2D tex0;
												   
    uniform float sat;
    uniform float brt;
    uniform float con;
												   
    uniform float rMult;
    uniform float gMult;
    uniform float bMult;
												   
    vec4 gammaCorrection(vec3 color, vec3 gamma){
        return vec4( pow(color.r, gamma.r), pow(color.g, gamma.g), pow(color.b, gamma.b), 1.0 );
    }
												   
    vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)
    {
        // Increase or decrease theese values to adjust r, g and b color channels seperately
        const float AvgLumR = 0.5;
        const float AvgLumG = 0.5;
        const float AvgLumB = 0.5;
													   
        const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
													   
        vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
        vec3 brtColor = color * brt;
        vec3 intensity = vec3(dot(brtColor, LumCoeff));
        vec3 satColor = mix(intensity, brtColor, sat);
        vec3 conColor = mix(AvgLumin, satColor, con);
        return conColor;
    }
												   
    void main(void){
        vec4 texture = texture2D(tex0, gl_TexCoord[0].st);
        gl_FragColor = vec4(ContrastSaturationBrightness(texture.rgb,brt,sat,con),texture.a);
        gl_FragColor.rgb = vec3(gl_FragColor.r*rMult, gl_FragColor.g*gMult, gl_FragColor.b*bMult);
    }
												   
    );
	
static string UnnormalizedFragShader = STRINGIFY(
    uniform sampler2DRect tex0;
													 
    uniform float sat;
    uniform float brt;
    uniform float con;
													 
    uniform float rMult;
    uniform float gMult;
    uniform float bMult;
													 
    vec4 gammaCorrection(vec3 color, vec3 gamma){
        return vec4( pow(color.r, gamma.r), pow(color.g, gamma.g), pow(color.b, gamma.b), 1.0 );
    }
													 
    vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)
    {
        // Increase or decrease theese values to adjust r, g and b color channels seperately
        const float AvgLumR = 0.5;
        const float AvgLumG = 0.5;
        const float AvgLumB = 0.5;
														 
        const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
														 
        vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
        vec3 brtColor = color * brt;
        vec3 intensity = vec3(dot(brtColor, LumCoeff));
        vec3 satColor = mix(intensity, brtColor, sat);
        vec3 conColor = mix(AvgLumin, satColor, con);
        return conColor;
    }
													 
    void main(void){													 
        vec4 texture = texture2DRect(tex0, gl_TexCoord[0].st);
        gl_FragColor = vec4(ContrastSaturationBrightness(texture.rgb,brt,sat,con),texture.a);
        gl_FragColor.rgb = vec3(gl_FragColor.r*rMult, gl_FragColor.g*gMult, gl_FragColor.b*bMult);														 
    }
													 
    );
}
