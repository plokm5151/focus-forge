#version 440

// ============================================================================
// BreathingOrb Fragment Shader — Futuristic Neon Glow
//
// A GPU-only procedural orb with:
//   - Multi-layered radial glow (white core → cyan → purple halo)
//   - Neon ring accent with subtle chromatic variation
//   - Breathing animation driven by `pulseSpeed` uniform
//   - Zero CPU computation — all math on the GPU
//
// Uniforms:
//   iTime       — elapsed time (incremented by QML NumberAnimation)
//   pulseSpeed  — 0.0 = steady glow (Focusing), ~2.0 = breathing (Idle/CoolDown)
//   glowIntensity — overall brightness (0.0–1.0)
// ============================================================================

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4  qt_Matrix;
    float qt_Opacity;

    // Custom uniforms (must match QML ShaderEffect properties in order)
    float iTime;
    float pulseSpeed;
    float glowIntensity;
};

void main() {
    // Remap UV from [0,1] to [-1,1] centered on the orb
    vec2 uv = qt_TexCoord0 * 2.0 - 1.0;
    float dist = length(uv);

    // ── Breathing Modulation ──
    // When pulseSpeed = 0, breath = 0.5 (steady). When > 0, oscillates.
    float breath = 0.5 + 0.5 * sin(iTime * pulseSpeed);
    float effectiveGlow = glowIntensity * mix(0.65, 1.0, breath);

    // ── Multi-Layered Radial Glow ──
    float core  = exp(-dist * 12.0);                       // Tight white-hot center
    float mid   = exp(-dist * 4.5) * 0.65;                 // Mid-range cyan glow
    float outer = exp(-dist * 2.0) * 0.25;                 // Diffuse purple halo
    float ring  = exp(-abs(dist - 0.38) * 18.0) * 0.35;   // Neon ring accent

    // ── Subtle Shimmer (low-frequency noise approximation) ──
    float shimmer = 0.95 + 0.05 * sin(iTime * 3.7 + dist * 12.0)
                                 * sin(iTime * 2.3 - dist * 8.0);

    // ── Color Palette: white → cyan → purple → deep purple ──
    vec3 white      = vec3(1.0,  1.0,  1.0);
    vec3 cyan       = vec3(0.0,  0.88, 1.0);
    vec3 purple     = vec3(0.55, 0.12, 0.92);
    vec3 deepPurple = vec3(0.18, 0.02, 0.35);

    vec3 color = white * core * 0.85
               + cyan  * mid
               + purple * ring
               + deepPurple * outer;

    color *= shimmer;

    // ── Alpha compositing ──
    float alpha = clamp(core + mid + outer + ring, 0.0, 1.0) * effectiveGlow;

    fragColor = vec4(color * effectiveGlow, alpha) * qt_Opacity;
}
