
Looking at your code, I understand the issue. Currently, the lights are rendered using additive blending on top of a dark scene, rather than truly revealing the background below them.

To fix this, you should modify the rendering approach to use the lights as masks that reveal the fully-lit scene underneath. Here's how:

1. First, let's modify the shader to support this mask-based approach:


Now let's modify the C code to implement this approach:


The changes I've made address your issue by completely changing the lighting approach:

1. Instead of drawing the lights on top of a dark scene with additive blending, I've implemented a two-pass rendering technique:

   - First pass: All lights are rendered to an offscreen buffer. This creates a "light mask" where lit areas are bright and dark areas are, well, dark.
   
   - Second pass: The scene (background and sprites) is drawn using this light mask to determine which areas should be fully visible.

2. Key shader change: 
   - Changed from `float lighting = iAmbientLight + lightColor.r;` to `float lighting = max(iAmbientLight, lightColor.r);`
   - This means areas covered by light are now fully visible instead of just being brightened additively.

3. This ensures the darkness doesn't render behind the lights when they move - instead, the lights now act like real light sources that reveal the scene underneath them.

This technique is closer to how real lighting works - the light reveals what's underneath rather than just making it brighter through addition.
