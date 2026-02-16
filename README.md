# Voxel Engine
A custom 3D Voxel Engine developed from scratch in C++. This project is a hobby project inspired by the sandbox mechanics of Minecraft.

# Features
- Procedural World Generation: Infinite Terran generation. The Algorithm for this is still basic and also needs caves and biomes.
- Frustum Culling: Only renders chunks in the cameras view to increase performance. Most recent update that split Solid and Transparent meshes into seperate Buffers however makes it slow.
- Face Culling: Prevents the rendering of hidden blocks faces. Also takes transparent blocks into account.
- Transparency Layer: Dedicated rendering pass for water and glass blocks.
- Skybox: Cubemap for the Sky
- Multithreading: Thread-Pool for async chunk generation and meshing for smooth traversal without frame drops.
- Raycasting: For Block manipulation

# Tech Stack
- C++17
- OpenGL 4.x
- GLEW/GLFW
- GLM: For Vector/Matrix math
- FastNoiseLite: For the World generation
- ImGui
- stb_image

# Performance
This project has taught me a lot about performance optimization and how tiny code changes can have a massive impact on the overall performance.

- **Memory Reservation:** One of the most significant fixes for stuttering/freezing was choosing and reserving a optimal amount of memory. Choosing to big of a numbers hurts performance and choosing to small of a number hurts performance with reallocations.
- **Efficient Object Construction:** How seemingly non significant changes like using `emplace_back` instead of `push_back` increases performance by avoiding copy and move operations.
- **Data Batching & Structs:** Instead of sending individual values, I optimized data transfer by packing attributes into structs. Moving data in batches significantly increased performance.
- **Asynchronous Workloads:** Implementing a Thread-Pool was essential for heavy computations (like Terrain-Generation and Meshing) to decouple them from the main render loop. This ensures the engine maintains a high FPS and doesnt freeze even while generating massive parts of the world in the background.

# Better Water Rendering (08.02.2026)
<img width="2559" height="1388" alt="Screenshot 2026-02-08 162919" src="https://github.com/user-attachments/assets/581ff7cf-a498-475d-a582-770a5d1d442a" />

# Exponential Height Fog (08.02.2026)
<img width="2559" height="1386" alt="image" src="https://github.com/user-attachments/assets/91cf4319-9a01-4469-ad56-736ca0003f7e" />



# Old Pictures
<img width="2559" height="1372" alt="Screenshot 2026-02-07 131623" src="https://github.com/user-attachments/assets/5f4dc38e-8654-4ff8-ae55-f261326f7bce" />
<img width="2559" height="1392" alt="Screenshot 2026-02-07 130258" src="https://github.com/user-attachments/assets/75c8b91a-b7c1-4002-83ec-75a7f6de375c" />
<img width="2559" height="1376" alt="Screenshot 2026-02-07 125801" src="https://github.com/user-attachments/assets/8b96574d-606f-4175-b559-a2b2712c6a90" />
<img width="2559" height="1382" alt="Screenshot 2026-02-07 125116" src="https://github.com/user-attachments/assets/72e36310-1faa-4f48-b871-351c908ccc20" />
<img width="2555" height="1380" alt="Screenshot 2026-02-07 124646" src="https://github.com/user-attachments/assets/a3667682-8cc6-4bad-ad30-d67ed7e91047" />
<img width="2559" height="1387" alt="Screenshot 2026-02-07 121446" src="https://github.com/user-attachments/assets/c90975c9-32d6-45d0-8e40-9b9a44b9010a" />
