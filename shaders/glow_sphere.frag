#version 330 core 
#define PI 3.1415926535897932

out vec4 FragColor;

in vec3 worldPos;
in vec3 normal;
in vec3 texCoord;

uniform mat4 model;
uniform sampler2D emission;
uniform float time;
uniform vec3 cameraPos;

vec3 grid[64] = vec3[64](
    vec3(0.3483, 0.5705, -0.7438), vec3(-0.3869, 0.6137, 0.6882), vec3(-0.6921, -0.7168, -0.08508), vec3(-0.3194, 0.9224, -0.2171),
    vec3(0.7384, -0.299, -0.6045), vec3(-0.3741, -0.9116, -0.1706), vec3(-0.7798, -0.4319, -0.4531), vec3(0.3475, 0.6152, 0.7077),
    vec3(0.7655, 0.3983, 0.5053), vec3(-0.3136, -0.4226, -0.8503), vec3(-0.04437, -0.5088, -0.8597), vec3(0.952, -0.3054, -0.02311),
    vec3(-0.1375, -0.55, -0.8238), vec3(-0.8729, -0.407, -0.269), vec3(0.4062, -0.5627, 0.72), vec3(0.7788, 0.6131, 0.1327),

    vec3(0.8799, -0.1368, 0.4551), vec3(-0.4991, 0.8332, -0.2381), vec3(-0.5974, -0.8016, -0.0259), vec3(0.7078, -0.6497, -0.2774),
    vec3(-0.05893, 0.8325, 0.5508), vec3(-0.5051, 0.1482, -0.8502), vec3(-0.3412, -0.3002, -0.8908), vec3(0.5733, 0.7941, 0.2019),
    vec3(-0.5069, -0.02774, -0.8616), vec3(-0.5971, 0.202, 0.7763), vec3(-0.1188, -0.3624, -0.9244), vec3(0.3634, -0.6654, -0.652),
    vec3(-0.4699, 0.7041, 0.5323), vec3(0.4603, 0.5772, 0.6746), vec3(-0.112, 0.6619, -0.7412), vec3(-0.6382, 0.6906, -0.3402),

    vec3(-0.398, -0.0515, -0.916), vec3(-0.444, 0.2248, -0.8674), vec3(-0.7021, 0.02239, 0.7117), vec3(-0.9646, 0.04746, -0.2592),
    vec3(0.4385, 0.3506, -0.8275), vec3(0.1629, -0.4968, 0.8525), vec3(-0.5173, -0.2474, -0.8193), vec3(0.1701, 0.3519, -0.9205),
    vec3(0.01274, -0.9753, -0.2206), vec3(-0.4915, 0.8511, 0.1846), vec3(0.44, 0.5095, 0.7395), vec3(0.4335, 0.03902, -0.9003),
    vec3(-0.1609, 0.8897, -0.4272), vec3(0.547, -0.732, -0.4061), vec3(-0.1312, 0.7051, 0.6969), vec3(0.6721, 0.6591, 0.3375),

    vec3(-0.7626, 0.05094, 0.6449), vec3(-0.9231, 0.2391, 0.3011), vec3(-0.1631, 0.9151, -0.3689), vec3(-0.5741, -0.7522, 0.3234),
    vec3(-0.7416, 0.4428, -0.5039), vec3(-0.581, 0.7136, 0.3913), vec3(0.8685, -0.01959, 0.4954), vec3(-0.6369, 0.1896, 0.7473),
    vec3(-0.5474, -0.6044, -0.5789), vec3(0.9854, 0.0805, 0.1498), vec3(0.8955, 0.4229, 0.1388), vec3(-0.0252, -0.6967, 0.7169),
    vec3(-0.4673, 0.4921, -0.7345), vec3(0.5237, -0.2602, 0.8112), vec3(0.8907, -0.1942, -0.4111), vec3(-0.4764, 0.3531, 0.8052)
);

float interp(float from, float to, float f)
{
        return mix(from, to, smoothstep(0.0, 1.0, f));   
}

vec3 grid_vec(ivec3 gridPoint)
{
    return grid[16 * (gridPoint.z % 4) + 4 * (gridPoint.y % 4) + (gridPoint.x % 4)];
}

float perlin3D(vec3 p)
{
    ivec3 grid000 = ivec3(floor(p));
    vec3 delta = fract(p);
    float xx[2] = float[2](0.0, 0.0);
    for (int dx = 0; dx <= 1; dx++) {
        float yy[2] = float[2](0.0, 0.0);
        for (int dy = 0; dy <= 1; dy++) {
            float zz[2] = float[2](0.0, 0.0);
            for (int dz = 0; dz <= 1; dz++) {
                ivec3 gridPoint = grid000 + ivec3(dx, dy, dz);
                vec3 gridVector = grid_vec(gridPoint);
                vec3 distVector = p - vec3(gridPoint);
                zz[dz] = dot(gridVector, distVector);
            }
            yy[dy] = interp(zz[0], zz[1], delta.z);
        }
        xx[dx] = interp(yy[0], yy[1], delta.y);
    }
    return interp(xx[0], xx[1], delta.x);
}

vec2 worley3D(vec3 p0)
{
    vec3 grid000 = ivec3(floor(p0));
    
    vec2 least = vec2(10.0);

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                vec3 gridCorner = vec3(grid000 + ivec3(dx, dy, dz));
                vec3 p = gridCorner + vec3(perlin3D(gridCorner + vec3(0.5, 2.5, 2.5)),
                                           perlin3D(gridCorner + vec3(2.5, 0.5, 2.5)),
                                           perlin3D(gridCorner + vec3(2.5, 2.5, 0.5)));

                float l = length(p - p0);

                if (l < least.x) {
                    least.y = least.x;
                    least.x = l;
                } else if (l < least.y) {
                    least.y = l;
                }
            }
        }
    }

    return least;
}

vec2 sphere_uv(vec3 direction)
{
    float u = 0.5 + atan(direction.z, direction.x) / (2 * PI);
    float v = 0.5 - asin(direction.y) / PI;

    return vec2(u, v);
}

float fractal3D(vec3 p)
{
    float value = 0.0;
    float weight = 0.5;
    float scale = 1.0;
    float frequency = 0.05;
    for (int i = 0; i < 5; i++) {
        value += weight * perlin3D(p * (scale + 0.125 * scale * sin(time * frequency)));
        weight *= 0.6;
        scale *= 4.0;
        frequency *= 1.2;
    }

    return value;
}

void main()
{

    //vec2 least = worley3D(vec3(time / 10.0f) + texCoord * 16);
    /*
    float value = fractal3D(texCoord);

    vec3 dark = vec3(0.4, 0.1, 0.01);
    vec3 light = vec3(0.9, 0.5, 0.05);
    vec3 white = vec3(1.0);

    vec3 col = vec3(0.0);
    if (value < 0) {
        col = mix(dark, light, value + 1.0);
    } else {
        col = mix(light, white, value);
    }

    FragColor = vec4(col, 1.0);

    */
    vec3 view = normalize(cameraPos - worldPos);
    vec3 nnormal = normalize(normal);
    vec3 col = vec3(texture(emission, sphere_uv(texCoord)));

    float f = 1.0 / dot(view, nnormal);
    f = 1.0 - 1.0 / (1 + f);
    f *= 2.0;
    FragColor = vec4(f * col, 1.0);
}
