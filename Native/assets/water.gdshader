shader_type spatial;
render_mode cull_disabled;
uniform vec4 water_color : source_color = vec4(0.08,0.34,0.4,1.0);
void vertex() {
 VERTEX.y += sin(VERTEX.x * 0.14 + TIME * 0.9) * 0.13 + sin(VERTEX.z * 0.19 + TIME * 0.65) * 0.1;
}
void fragment() {
 float ripples = sin(UV.x * 480.0 + TIME) * sin(UV.y * 320.0 - TIME * 0.7);
 ALBEDO = water_color.rgb + vec3(0.025) * ripples;
 ROUGHNESS = 0.28;
 METALLIC = 0.25;
 SPECULAR = 0.65;
}
