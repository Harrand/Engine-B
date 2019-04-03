#include "core/listener.hpp"
#include "physics/physics.hpp"
#include "graphics/asset.hpp"
#include "graphics/gui/button.hpp"
#include "graphics/gui/textfield.hpp"
#include "core/scene.hpp"
#include "graphics/skybox.hpp"
#include "core/topaz.hpp"
#include "utility/time.hpp"
#include "graphics/frame_buffer.hpp"
#include "graphics/animated_texture.hpp"
#include "data/scene_importer.hpp"

void init();

int main()
{
    tz::initialise();
    init();
    tz::terminate();
    return 0;
}

void init()
{
    Window wnd("Engine B - MSOC Demo", 0, 30, 1920, 1080);
    wnd.set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);

    Font font("../../../res/runtime/fonts/Comfortaa-Regular.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2I{100, 50}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, ProgressBarTheme{{{0.5f, {0.0f, 0.0f, 1.0f}}, {1.0f, {1.0f, 0.0f, 1.0f}}}, {0.1f, 0.1f, 0.1f}}, 0.5f);

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    auto make_button = [&wnd, &font](Vector2I position, std::string title)->Button&{return wnd.emplace_child<Button>(position, Vector2I{100, 50}, font, Vector3F{}, title, Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});};

    Button& wireframe_button = wnd.emplace_child<Button>(Vector2I{0, 100}, Vector2I{100, 50}, font, Vector3F{}, "Toggle Wireframes", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& maze1_button = make_button({0, 150}, "Maze 1");
    Button& maze2_button = make_button({0, 200}, "Maze 2");
    Button& maze3_button = make_button({0, 250}, "Maze 3");
    Button& dynamic1_button = make_button({0, 300}, "Dynamic 1");
    Button& dynamic2_button = make_button({0, 350}, "Dynamic 2");
    Button& dynamic3_button = make_button({0, 400}, "Dynamic 3");
    Button& terrain1_button = make_button({0, 450}, "Terrain 1");
    bool wireframe = false;
    wireframe_button.set_callback([&wireframe](){wireframe = !wireframe;});

    constexpr float speed = 0.5f;
    Shader render_shader("../../../src/shaders/3D_FullAssets");

    Shader gui_shader("../../../src/shaders/Gui");
    Shader hdr_gui_shader("../../../src/shaders/Gui_HDR");
    Camera camera;
    camera.position = {0, 0, 0};

    SceneImporter importer1{"maze1.xml"};
    SceneImporter importer2{"maze2.xml"};
    SceneImporter importer3{"maze3.xml"};
    SceneImporter importer4{"terrain1.xml"};
    Scene scene1 = importer1.retrieve();
    Scene scene2 = importer2.retrieve();
    Scene scene3 = importer3.retrieve();
    Scene dynamic1 = importer1.retrieve();
    Scene dynamic2 = importer2.retrieve();
    Scene dynamic3 = importer3.retrieve();
    Scene terrain1 = importer4.retrieve();
    Scene* scene = &scene1;

    const std::vector<Vector3F> maze1_midpoints{Vector3F{210.0f, 100.0f, -90.0f}, Vector3F{730.0f, 100.0f, -90.0f}, Vector3F{730.0f, 100.0f, -310.0f}, Vector3F{320.0f, 100.0f, -320.0f}, Vector3F{110.0f, 100.0f, -410.0f}, Vector3F{420.0f, 100.0f, -510.0f}, Vector3F{720.0f, 100.0f, -610.0f}, Vector3F{930.0f, 100.0f, -620.0f}, Vector3F{420.0f, 100.0f, -810.0f}, Vector3F{110.0f, 100.0f, -920.0f}, Vector3F{430.0f, 100.0f, -1120.0f}, Vector3F{730.0f, 100.0f, -1020.0f}, Vector3F{940.0f, 100.0f, -1020.0f}};

    const std::vector<Vector3F> maze2_midpoints{Vector3F{1100.0f, 1000.0f, -900.0f}, Vector3F{3200.0f, 1000.0f, -999.996f}, Vector3F{5300.0f, 1000.0f, -1000.0f}, Vector3F{4300.0f, 1000.0f, -3300.0f}, Vector3F{1100.0f, 1000.0f, -3200.0f}, Vector3F{2100.0f, 1000.0f, -5500.0f}, Vector3F{5300.0f, 1000.0f, -5500.0f}};

    const std::vector<Vector3F> maze3_midpoints{Vector3F{9, 10, -9}, Vector3F{41, 10, -9}, Vector3F{71, 10, -9}, Vector3F{101, 10, -9}, Vector3F{141, 10, -9}, Vector3F{151, 10, -40}, Vector3F{130, 10, -39}, Vector3F{100, 10, -29}, Vector3F{51, 10, -29}, Vector3F{10, 10, -29}, Vector3F{9, 10, -60}, Vector3F{41, 10, -50}, Vector3F{91, 10, -60}, Vector3F{151, 10, -80}, Vector3F{131, 10, -80}, Vector3F{100, 10, -90}, Vector3F{40, 10, -80}, Vector3F{10, 10, -110}, Vector3F{91, 10, -110}, Vector3F{101, 10, -130}, Vector3F{31, 10, -140}, Vector3F{9, 10, -150}, Vector3F{80, 10, -160}, Vector3F{130, 10, -170}, Vector3F{151, 10, -160}, Vector3F{151, 10, -200}, Vector3F{19, 10, -170}, Vector3F{29, 10, -190}, Vector3F{69, 10, -211}, Vector3F{90, 10, -190}};

    std::vector<Vector3F> terrain1_midpoints;
    for(float x = 0; x < 220; x += 20)
    {
        for(float y = 0; y < 220; y += 20)
        {
            terrain1_midpoints.emplace_back(x, 1.0f, y);
        }
    }

    auto get_height = [&]()->float
    {
        if(scene == &scene1 || scene == &dynamic1)
            return 100.0f;
        else if(scene == &scene2 || scene == &dynamic2)
            return 1000.0f;
        else if(scene == &scene3 || scene == &dynamic3)
            return 10.0f;
        else
            return 0.0f;
    };

    auto get_correct_midpoints = [&]()->const std::vector<Vector3F>&
    {
        if(scene == &scene1 || scene == &dynamic1)
            return maze1_midpoints;
        else if(scene == &scene2 || scene == &dynamic2)
            return maze2_midpoints;
        else if(scene == &scene3 || scene == &dynamic3)
            return maze3_midpoints;
        else
            return maze1_midpoints;
    };

    bool dynamic = false;

    maze1_button.set_callback([&scene, &scene1, &camera, &dynamic](){scene = &scene1; camera.position = {110, 100, -110}; dynamic = false;});
    maze2_button.set_callback([&scene, &scene2, &camera, &dynamic](){scene = &scene2; camera.position = {1100, 1000, -1100}; dynamic = false;});
    maze3_button.set_callback([&scene, &scene3, &camera, &dynamic](){scene = &scene3; camera.position = {11, 10, -11}; dynamic = false;});

    dynamic1_button.set_callback([&scene, &dynamic1, &camera, &dynamic](){scene = &dynamic1; camera.position = {110, 100, -110}; dynamic = true;});
    dynamic2_button.set_callback([&scene, &dynamic2, &camera, &dynamic](){scene = &dynamic2; camera.position = {1100, 1000, -1100}; dynamic = true;});
    dynamic3_button.set_callback([&scene, &dynamic3, &camera, &dynamic](){scene = &dynamic3; camera.position = {11, 10, -11}; dynamic = true;});

    terrain1_button.set_callback([&scene, &terrain1, &camera, &dynamic](){scene = &terrain1; camera.position = {}; dynamic = false;});

    AssetBuffer assets;
    assets.emplace<Mesh>("cube_lq", "../../../res/runtime/models/cube.obj");
    assets.emplace<Mesh>("cube", "../../../res/runtime/models/cube_hd.obj");
    assets.emplace<Mesh>("monkey", "../../../res/runtime/models/monkeyhead.obj");
    assets.emplace<Mesh>("cylinder", "../../../res/runtime/models/cylinder.obj");
    assets.emplace<Mesh>("sphere", "../../../res/runtime/models/sphere.obj");
    assets.emplace<Mesh>("plane_hd", "../../../res/runtime/models/plane_hd.obj");
    assets.emplace<Texture>("bricks", "../../../res/runtime/textures/bricks.jpg");
    assets.emplace<Texture>("stone", "../../../res/runtime/textures/stone.jpg");
    assets.emplace<Texture>("wood", "../../../res/runtime/textures/wood.jpg");
    assets.emplace<NormalMap>("bricks_normal", "../../../res/runtime/normalmaps/bricks_normalmap.jpg");
    assets.emplace<NormalMap>("stone_normal", "../../../res/runtime/normalmaps/stone_normalmap.jpg");
    assets.emplace<NormalMap>("wood_normal", "../../../res/runtime/normalmaps/wood_normalmap.jpg");
    assets.emplace<ParallaxMap>("bricks_parallax", "../../../res/runtime/parallaxmaps/bricks_parallax.jpg");
    assets.emplace<ParallaxMap>("stone_parallax", "../../../res/runtime/parallaxmaps/stone_parallax.png", 0.06f, -0.5f);
    assets.emplace<ParallaxMap>("wood_parallax", "../../../res/runtime/parallaxmaps/wood_parallax.jpg");
    assets.emplace<DisplacementMap>("bricks_displacement", "../../../res/runtime/displacementmaps/bricks_displacement.png");

    long long int time = tz::utility::time::now();
    std::vector<float> msoc_deltas;
    Timer second_timer, tick_timer;
    TimeProfiler profiler;
    using namespace tz::graphics;
    while(!wnd.is_close_requested())
    {
        profiler.begin_frame();
        static float x = 0;
        progress.set_progress((1 + std::sin(x += 0.01)) / 2.0f);
        second_timer.update();
        tick_timer.update();
        static bool should_report = false;
        if(second_timer.millis_passed(1000.0f))
        {
            using namespace tz::utility::generic::cast;
            label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            second_timer.reload();
            // print out time deltas for profiling.
            static std::size_t cumulative_frame_id;
            if(should_report)
            {
                for (std::size_t frame_id = 0; frame_id < profiler.deltas.size(); frame_id++)
                {
                    //std::cout << (/*cumulative_frame_id + */frame_id) << ", " << profiler.deltas[frame_id] << "\n"; // total cpu + gpu
                    std::cout << (/*cumulative_frame_id + */frame_id) << ", " << msoc_deltas[frame_id] << "\n"; // msoc only
                }
                cumulative_frame_id += profiler.deltas.size();
                should_report = false;
            }
            profiler.reset();
        }

        long long int delta_time = tz::utility::time::now() - time;
        time = tz::utility::time::now();

        if(wireframe)
            tz::graphics::enable_wireframe_render(true);
        wnd.set_render_target();
        wnd.clear();
        scene->render(&render_shader, &gui_shader, camera, {wnd.get_width(), wnd.get_height()});
        msoc_deltas.push_back(scene->get_msoc_time_this_frame(true));
        constexpr int tps = 120;
        constexpr float tick_delta = 1000.0f / tps;
        if(tick_timer.millis_passed(tick_delta))
        {
            scene->update(tick_delta / 1000.0f);
            if(dynamic)
            {
                // if we're in scene2, continue moving all the objects around.
                static int x = 0;
                for(const StaticObject& object : scene->get_static_objects())
                    object.transform.scale.y = get_height() * std::abs(std::sin(std::cbrt(3 + static_cast<int>(object.transform.position.x) % static_cast<int>(scene->get_boundary().get_maximum().x - scene->get_boundary().get_minimum().x)) * ++x * 0.0002f));
            }
            tick_timer.reload();
        }
        if(wireframe)
            tz::graphics::enable_wireframe_render(false);
        wnd.update(gui_shader, &hdr_gui_shader);

        if(mouse_listener.is_left_clicked())
        {
            Vector2F delta = mouse_listener.get_mouse_delta_position();
            camera.rotation.y += 0.03 * delta.x;
            camera.rotation.x += 0.03 * delta.y;
            mouse_listener.reload_mouse_delta();
        }
        auto estimate_camera_node_id = [&]()->std::size_t
        {
            Vector3F closest_node_midpoint = get_correct_midpoints().front();
            std::size_t closest_node_id = 0;
            for(std::size_t node_id = 0; node_id < get_correct_midpoints().size(); node_id++)
            {
                if((get_correct_midpoints()[node_id] - camera.position).length() < (closest_node_midpoint - camera.position).length())
                {
                    closest_node_id = node_id;
                    closest_node_midpoint = get_correct_midpoints()[node_id];
                }
            }
            return closest_node_id;
        };
        if(key_listener.is_key_pressed("Escape"))
            break;
        if(key_listener.is_key_pressed("W"))
            camera.position += camera.forward() * delta_time * speed;
        if(key_listener.is_key_pressed("S"))
            camera.position += camera.backward() * delta_time * speed;
        if(key_listener.is_key_pressed("A"))
            camera.position += camera.left() * delta_time * speed;
        if(key_listener.is_key_pressed("D"))
            camera.position += camera.right() * delta_time * speed;
        static std::size_t terrain_node_counter = 0;
        if(key_listener.catch_key_pressed("P"))
        {
            if(scene == &terrain1)
            {
                if(--terrain_node_counter < 0)
                    terrain_node_counter = terrain1_midpoints.size() - 1;
                camera.position = terrain1_midpoints[terrain_node_counter];
            }
            else
            {
                std::size_t node_id = estimate_camera_node_id();
                if (node_id == 0)
                    node_id = get_correct_midpoints().size() - 1;
                else
                    node_id--;
                camera.position = get_correct_midpoints()[node_id];
            }
        }
        if(key_listener.catch_key_pressed("N"))
        {
            if(scene == &terrain1)
            {
                if(++terrain_node_counter >= terrain1_midpoints.size())
                    terrain_node_counter = 0;
                camera.position = terrain1_midpoints[terrain_node_counter];
            }
            else
            {
                std::size_t node_id = estimate_camera_node_id();
                if (node_id >= get_correct_midpoints().size() - 1)
                    node_id = 0;
                else
                    node_id++;
                camera.position = get_correct_midpoints()[node_id];
            }
        }
        if(key_listener.catch_key_pressed("R"))
        {
            should_report = true;
        }
        profiler.end_frame();
    }
}