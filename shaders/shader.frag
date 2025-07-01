#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 diffuseColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 fragPos;
layout(location = 5) in vec2 tileStart;
layout(location = 6) in vec2 tileSize;
layout(location = 7) in vec2 repeatCount;

layout(push_constant) uniform MaterialData {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 emissionColor;
    float shininess;
    float opacity;
    float refractiveIndex;
    int illuminationModel;
    int hasTexture;
} material;


layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {

        vec2 tiledUV = fract(fragTexCoord * repeatCount);
        vec2 finalUV = tileStart + tiledUV * tileSize;
        
        if(fragNormal.x <0 && fragNormal.y <0 && fragNormal.z <0){
            if(material.hasTexture == 1) {
                vec4 texColor = texture(texSampler, finalUV);
                if(texColor.a < 0.1){
                    discard;
                }
                outColor = texColor * diffuseColor;
            }else{
                outColor = diffuseColor;
            }
        }else{
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(-vec3(-0.5, -1.0, -0.3));
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 0.9);

        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
         float spec = 0.0;
        if(diff > 0.0)
            spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = material.specularColor * spec * vec3(1.0, 1.0, 0.9);

        vec3 ambient = 0.3 * vec3(1.0, 1.0, 0.9);

        vec3 litColor = vec3(0);
        
        if(material.illuminationModel == 0) {
        litColor = diffuseColor.rgb;
        } else if(material.illuminationModel == 1) {
        litColor = diffuseColor.rgb * (ambient + diffuse);
        } else if(material.illuminationModel == 2) {
        litColor = diffuseColor.rgb * (ambient + diffuse + specular);
        } else if(material.illuminationModel == 3) {
        litColor = diffuseColor.rgb * (ambient + diffuse + specular + material.emissionColor);
        } else {
        litColor = diffuseColor.rgb * (ambient + diffuse + specular);
         }
         
         if(material.hasTexture == 1) {
            vec4 texColor = texture(texSampler, finalUV);
            litColor *= texColor.rgb;
        }
        outColor = vec4(litColor, diffuseColor.a);
        }

}