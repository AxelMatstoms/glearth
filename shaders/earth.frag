#version 330 core 
#define PI 3.1415926535897932

out vec4 FragColor;

in vec3 worldPos;
in vec3 normal;
in vec3 texCoord;

struct Light {
    vec3 position;
    
    vec3 emission;
};

uniform sampler2D daySide;
uniform sampler2D nightSide;
uniform sampler2D roughnessMap;
uniform sampler2D clouds;
uniform sampler2D normalMap;

uniform vec3 cameraPos;
uniform Light sun;
uniform bool normalMapping;
uniform bool orenNayar;

float fresnel(vec3 normal, vec3 view)
{
    /* float n1 = 1.0; float n2 = 1.5;
    
    float R0 = (n1 - n2) / (n1 + n2);
    R0 = R0 * R0;
    */
    float R0 = 0.04;

    float f = 1 - dot(normal, view);
    f = f * f * f * f * f;
    float R = R0 + (1 - R0) * f;

    return clamp(R, 0.0, 1.0);
}

float ndf_ggx(float alpha, vec3 normal, vec3 micronormal)
{
    float numerator = alpha * alpha;

    float n_dot_m = dot(normal, micronormal);
    float base = (n_dot_m * n_dot_m) * (alpha * alpha - 1.0) + 1.0;
    float denominator = PI * base * base;

    return numerator / denominator;
}

float g_ggx(float alpha, vec3 normal, vec3 view)
{
    float n_dot_v = dot(normal, view);

    float numerator = 2.0 * n_dot_v;

    float alpha2 = alpha * alpha;
    
    float denominator = n_dot_v + sqrt(alpha2 + (1 - alpha2) * n_dot_v * n_dot_v);

    return numerator / denominator;
}

float g_ggx_smith(float alpha, vec3 normal, vec3 view, vec3 light)
{
    return g_ggx(alpha, normal, view) * g_ggx(alpha, normal, light);
}

vec3 cook_torrance_brdf(float alpha, vec3 normal, vec3 view, vec3 light)
{
    vec3 half_vector = normalize(view + light);
    return ndf_ggx(alpha, normal, half_vector) * vec3(fresnel(half_vector, view)) * g_ggx_smith(alpha, normal, view, light) / (4.0 * dot(normal, light) * dot(normal, view));
}

float oren_nayar(float sigma, vec3 normal, vec3 view, vec3 light)
{
    if (!orenNayar) return 1.0;

    float sigma2 = sigma * sigma;
    float A = 1 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * (sigma2 / (sigma2 + 0.09));
    float cos_i = dot(normal, light);
    float cos_r = dot(normal, view);
    vec3 alpha;
    vec3 beta;
    if (cos_i < cos_r) {
        alpha = light;
        beta = view;
    } else {
        alpha = view;
        beta = light;
    }
    float sin_alpha = length(cross(alpha, normal));
    float tan_beta = length(cross(beta, normal)) / dot(beta, normal);

    vec3 view_proj_plane = normalize(view - dot(view, normal) * normal);
    vec3 light_proj_plane = normalize(light - dot(light, normal) * normal);

    return A + (B * max(0.0, dot(view_proj_plane, light_proj_plane)) * sin_alpha * tan_beta);
}

vec2 sphere_uv(vec3 direction)
{
    float u = 0.5 + atan(direction.z, direction.x) / (2 * PI);
    float v = 0.5 - asin(direction.y) / PI;

    return vec2(1.0 - u, v);
}

void main()
{
    vec3 lightDir = normalize(sun.position - worldPos);
    vec3 viewDir = normalize(cameraPos - worldPos);

    vec2 uv = sphere_uv(texCoord);

    vec3 nnormal = normalize(normal);

    vec3 axis = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(axis, nnormal));
    vec3 bitangent = cross(normal, tangent);

    mat3 tbn = mat3(-tangent, bitangent, -normal);
    

    vec3 cloud = vec3(texture(clouds, uv));

    if (normalMapping) {
        nnormal = normalize(tbn * mix(texture(normalMap, uv).rgb * vec3(2.0, 2.0, -1.0) - vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, -1.0), cloud));
    }

    float cos_theta = max(0.0, dot(nnormal, lightDir));
    
    vec3 night = vec3(texture(nightSide, uv));
    vec3 cloud_color = vec3(0.05, 0.05, 0.1);
    night = mix(night, cloud * cloud_color, cloud);

    night *= 0.25;

    vec3 day = vec3(texture(daySide, uv));
    day = mix(day, cloud, cloud);
    vec3 roughness_vector = vec3(texture(roughnessMap, uv));
    roughness_vector = mix(roughness_vector, vec3(1), cloud);
    float roughness = dot(roughness_vector, roughness_vector) / 3.0;
    float alpha = roughness * roughness;

    float F = fresnel(nnormal, normalize(mix(viewDir, nnormal, roughness)));
    
    vec3 diffuse = oren_nayar(alpha, nnormal, viewDir, lightDir) * day / PI;
    vec3 specular = cook_torrance_brdf(alpha, nnormal, viewDir, lightDir);

    vec3 brdf = mix(diffuse, specular, F);

    FragColor = vec4(mix(night, brdf * sun.emission, cos_theta), 1.0);
}
