#version 330

uniform float time;

in vec4 vpoint_mv;
in vec3 light_dir, view_dir;
uniform sampler2D texrefl, texrefr, texh, tex_dudv, tex_normal;
uniform vec3 Eye = vec3(2.0f,2.0f,4.0f);

out vec3 color;
in vec2 uv;
in vec4 pos;

// Constants //
float kDistortion = 0.015;
float kReflection = 0.01;
vec4 baseColor = vec4(0.0,0.05,1.0,1.0);

vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
vec4 lightNormal = vec4(0.0, 1.0, 0.0, 0.0);
vec4 biTangent = vec4(0.0, 0.0, 1.0, 0.0);

void main() {
        // Light Tangent Space //
        vec4 lightDirection = normalize(vec4(light_dir.xyz, 1.0));
        vec4 lightTanSpace = normalize(vec4(dot(lightDirection, tangent), dot(lightDirection, biTangent), dot(lightDirection, lightNormal), 1.0));

        // Fresnel Term //
        vec4 distOffset = texture(tex_dudv, uv + vec2(time/5)) * kDistortion;
        vec4 normal = texture(tex_normal, vec2(uv + distOffset.xy));
        normal = normalize(normal * 2.0 - 1.0);
        normal.a = 0.81;

        vec4 lightReflection = normalize(reflect(-1 * lightTanSpace, normal));
        vec4 invertedFresnel = vec4(dot(normal, lightReflection));
        vec4 fresnelTerm = 1.0 - invertedFresnel;

        // Reflection //
        vec4 dudvColor = texture(tex_dudv, vec2(uv + distOffset.xy));
        dudvColor = normalize(dudvColor * 2.0 - 1.0) * kReflection;

        // Projection Coordinates from http://www.bonzaisoftware.com/tnp/gl-water-tutorial/
        vec4 tmp = vec4(1.0 / pos.w);
        vec4 projCoord = pos * tmp;
        projCoord += vec4(1.0);
        projCoord *= vec4(0.5);
        projCoord += dudvColor;
        projCoord = clamp(projCoord, 0.001, 0.999);

        vec4 reflectionColor = mix(texture(texrefl, projCoord.xy), baseColor, 0.3);
        reflectionColor *= fresnelTerm;

        // Refraction //
        vec4 refractionColor = texture(texrefr, projCoord.xy);
        vec4 depthValue = vec4(0.1, 0.1, 0.1, 1.0);
        vec4 invDepth = 1.0 - depthValue;

        refractionColor *= invertedFresnel * invDepth;
        refractionColor += baseColor * depthValue * invertedFresnel;

        color = mix(reflectionColor,refractionColor,0.3).rgb;
        //color = (reflectionColor+refractionColor).rgb;









//	float h = texture(texh,uv).r;

//	vec2 proj_refl = vec2(0.0,0.0);
//	vec2 pos = uv*2 - vec2(1.0,1.0);
//	// -h is the height of reflected part
//        proj_refl.x = pos.x - h*(pos.x-Eye.x)/(Eye.z+h);
//        proj_refl.y = pos.y - h*(pos.y-Eye.y)/(Eye.z+h);
//        proj_refl = (proj_refl + vec2(1.0,1.0))*0.5;

//	vec3 reflectioncolor = texture(texrefl, proj_refl).rgb;

//        //refraction
//        proj_refl.x = pos.x + h*(pos.x-Eye.x)/(Eye.z-h);
//        proj_refl.y = pos.y + h*(pos.y-Eye.y)/(Eye.z-h);
//        proj_refl = (proj_refl + vec2(1.0,1.0))*0.5;
//        vec3 refractioncolor = texture(texrefr, proj_refl).rgb;
//        vec3 depthValue = vec3(0.1, 0.1, 0.1);
//        vec3 invDepth = 1.0 - depthValue;

//       // refractioncolor *= invDepth;
//       // refractioncolor += basecolor;

//    color = texture(tex_dudv,uv).rgb;//refractioncolor;

}

