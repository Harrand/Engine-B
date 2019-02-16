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
    Window wnd("Engine B - Demo", 0, 30, 1920, 1080);
    wnd.set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);

    Font font("../../../res/runtime/fonts/Comfortaa-Regular.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2I{100, 50}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, ProgressBarTheme{{{0.5f, {0.0f, 0.0f, 1.0f}}, {1.0f, {1.0f, 0.0f, 1.0f}}}, {0.1f, 0.1f, 0.1f}}, 0.5f);

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    Button& wireframe_button = wnd.emplace_child<Button>(Vector2I{0, 100}, Vector2I{100, 50}, font, Vector3F{}, "Toggle Wireframes", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& scene1_button = wnd.emplace_child<Button>(Vector2I{0, 150}, Vector2I{100, 50}, font, Vector3F{}, "Load Scene 1", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& scene2_button = wnd.emplace_child<Button>(Vector2I{0, 200}, Vector2I{100, 50}, font, Vector3F{}, "Load Scene 2", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& scene3_button = wnd.emplace_child<Button>(Vector2I{0, 250}, Vector2I{100, 50}, font, Vector3F{}, "Load Scene 3", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    bool wireframe = false;
    wireframe_button.set_callback([&wireframe](){wireframe = !wireframe;});

    constexpr float speed = 0.5f;
    Shader render_shader("../../../src/shaders/3D");

    Shader gui_shader("../../../src/shaders/Gui");
    Shader hdr_gui_shader("../../../src/shaders/Gui_HDR");
    Camera camera;
    camera.position = {0, 0, 0};

    SceneImporter importer1{"scene1.xml"};
    SceneImporter importer2{"scene2.xml"};
    SceneImporter importer3{"scene3.xml"};
    Scene scene1 = importer1.retrieve();
    Scene scene2 = importer2.retrieve();
    Scene scene3 = importer3.retrieve();
    Scene* scene = &scene1;

    scene1_button.set_callback([&scene, &scene1](){scene = &scene1;});
    scene2_button.set_callback([&scene, &scene2](){scene = &scene2;});
    scene3_button.set_callback([&scene, &scene3](){scene = &scene3;});

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
        if(second_timer.millis_passed(1000.0f))
        {
            using namespace tz::utility::generic::cast;
            label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            second_timer.reload();
            profiler.reset();
        }

        long long int delta_time = tz::utility::time::now() - time;
        time = tz::utility::time::now();

        if(wireframe)
            tz::graphics::enable_wireframe_render(true);
        wnd.set_render_target();
        wnd.clear();
        scene->render(&render_shader, &gui_shader, camera, {wnd.get_width(), wnd.get_height()});
        constexpr int tps = 120;
        constexpr float tick_delta = 1000.0f / tps;
        if(tick_timer.millis_passed(tick_delta))
        {
            scene->update(tick_delta / 1000.0f);
            if(scene == &scene2)
            {
                // if we're in scene2, continue moving all the objects around.
                static int x = 0;
                for(const StaticObject& object : scene->get_static_objects())
                    object.transform.scale.y = std::abs(100.0f * std::sin(std::cbrt(object.transform.position.x + 20) * ++x * 0.0001f));
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
        profiler.end_frame();
    }
}