#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 450
#define SPEED 2.0f
#define START_CAP 5 
#define RAYS_AMOUNT 60
#define RADIUS 25
#define MAIN_COLOR RAYWHITE
#define SELECTED_CIRCLE_COLOR LIGHTGRAY
#define BACKCOLOR BLACK
#define SELECTED_CIRCLE_RADIUS 30
#define MAX_RANDOM_SPEED 30
#define BOUNCE_COEFF 0.8
#define RAY_LEN (sqrt(WIDTH * WIDTH + HEIGHT * HEIGHT))

struct circle
{
    Vector2 centre;
    float radius;
    Color color;
    Vector2 acceleration;
    Vector2 speed;
} typedef circle;

struct circle_array
{
    circle* circles;
    int len;
    int capacity;
} typedef circle_array;


void handle_move(circle* selected_circle);
void handle_add_circle(circle_array* array, float radius, Color color);
void free_circles_if_pressed(circle_array* arr);
void draw_ray(Vector2 start_point, double len, double angle, const circle_array* col_objs, Color color);
void draw_rays(Vector2 start_point, int amount_rays, const circle_array* c, Color color);
void add_circle(circle_array* array, Vector2 point, float radius, Color color);
void check_collision_w_border(circle* c, Sound* s);
void handle_physic(circle_array* arr, circle* selected_circle, Sound* s);
void handle_position(circle_array* arr);
void set_new_speed_vector(circle* crl1, circle* crl2, double dist);
void check_collision_w_circles(circle_array* arr, int i, Sound* s);
void change_selected_circle(circle_array* arr, circle** current_c_ptr, circle* main_c);
void create_chaos(circle* c);
void handle_chaos(circle_array* arr);
int check_ray_collision_w_circle(Vector2 point, const circle_array* col_objs);


void handle_move(circle* selected_circle){
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selected_circle != NULL){
            Vector2 mouse_coords = {GetMouseX(), GetMouseY()};
            selected_circle->centre.x = GetMouseX();
            selected_circle->centre.y = GetMouseY();
        }
}

void handle_add_circle(circle_array* array, float radius, Color color){
    if (IsKeyPressed(KEY_SPACE)){
        Vector2 mouse_coords = {GetMouseX(), GetMouseY()};
        add_circle(array, mouse_coords, radius, color);
    }
}

void free_circles_if_pressed(circle_array* arr){
    if (IsKeyPressed(KEY_BACKSPACE)){
        free(arr->circles);
        arr->len = 0;
        arr->capacity = START_CAP;
        arr->circles = malloc(sizeof(circle) * arr->capacity);
    }
}

int check_ray_collision_w_circle(Vector2 point, const circle_array* col_objs){
    for (int i = 0; i < col_objs->len; i++)
        {
            if (Vector2Distance(point, col_objs->circles[i].centre) <= col_objs->circles[i].radius)
                return 1;
        }
    return 0;
}

void draw_ray(Vector2 start_point, double len, double angle, const circle_array* col_objs, Color color){
    Vector2 point = start_point;
    for (double r = 0; r < len; r += 0.5f)
    {
        if (check_ray_collision_w_circle(point, col_objs))
                break;
        point.x = start_point.x + r * cos(angle);
        point.y = start_point.y + r * sin(angle);
        DrawPixelV(point, color);   
    }
}

void draw_rays(Vector2 start_point, int amount_rays, const circle_array* c, Color color){
    double angle_delta = (2 * PI) / amount_rays;
    double stop = RAY_LEN;
    for (int t = 0; t < amount_rays; t++)
        draw_ray(start_point, stop, t * angle_delta, c, color);   
}

void add_circle(circle_array* array, Vector2 point, float radius, Color color){
    array->circles[array->len].centre = point;
    array->circles[array->len].radius = radius;
    array->circles[array->len].speed.x = 0;
    array->circles[array->len].speed.y = 0;
    array->circles[array->len].acceleration.x = 0;
    array->circles[array->len].acceleration.y = 0.2f;
    array->circles[array->len++].color = color;
    if (array->len >= array->capacity - 1){
        array->capacity *= 2;
        array->circles = realloc(array->circles, array->capacity * sizeof(circle));
    }
}

void check_collision_w_border(circle* c, Sound* s){
    if ((c->centre.x + c->radius) > WIDTH || (c->centre.x - c->radius) < 0){
        c->speed.x *= (-1 * BOUNCE_COEFF);
        if (fabs(c->speed.x) > (fabs(SPEED) / 5)) //MAGIC CONSTANT TO PRPEVENT AWFUL SOUND
            PlaySound(*s);
    }
    if ((c->centre.y + c->radius) > HEIGHT || (c->centre.y - c->radius) < 0){
        c->speed.y *= (-1 * BOUNCE_COEFF);
        if (fabs(c->speed.y) > (fabs(SPEED) / 5)) //MAGIC CONSTANT TO PRPEVENT AWFUL SOUND
            PlaySound(*s);
    }
}

void handle_physic(circle_array* arr, circle* selected_circle, Sound* s){
    for (int i = 0; i < arr->len; i++)
    {
        if(&(arr->circles[i]) != selected_circle){
            check_collision_w_circles(arr, i, s); 
            arr->circles[i].speed = Vector2Add(arr->circles[i].speed, arr->circles[i].acceleration);
            arr->circles[i].centre = Vector2Add(arr->circles[i].speed, arr->circles[i].centre);
            check_collision_w_border(&(arr->circles[i]), s);
            handle_position(arr);
        }
    }
}

void handle_position(circle_array* arr){
    for (int i = 0; i < arr->len; i++)
    {
        circle* c = &(arr->circles[i]);
        if ((c->centre.x + c->radius) > WIDTH){
            c->centre.x = WIDTH - c->radius;
        }
        if ((c->centre.x - c->radius) < 0){
            c->centre.x = c->radius;
        }
        if ((c->centre.y + c->radius) > HEIGHT){
            c->centre.y = HEIGHT - c->radius;
        }
        if ((c->centre.y - c->radius) < 0){
            c->centre.y = c->radius;
        }
    }
}

void set_new_speed_vector(circle* crl1, circle* crl2, double dist){
    float overlap;
    Vector2 v1 = crl1->speed;
    Vector2 v2 = crl2->speed;
    Vector2 c1 = crl1->centre;
    Vector2 c2 = crl2->centre;

    Vector2 normal = Vector2Normalize(Vector2Subtract(c2, c1));

    float s1 = -0.85 * Vector2DotProduct(Vector2Subtract(v1, v2), Vector2Subtract(c1, c2)) / (dist * dist);
    float s2 = -0.85 * Vector2DotProduct(Vector2Subtract(v2, v1), Vector2Negate(Vector2Subtract(c1, c2))) / (dist * dist);

    if (dist < (crl1->radius + crl2->radius))
    {   
        crl1->speed = Vector2Add(v1, Vector2Scale(Vector2Subtract(c1, c2), s1));
        crl2->speed = Vector2Add(v2, Vector2Scale(Vector2Negate(Vector2Subtract(c1, c2)), s2));

        overlap = (crl1->radius + crl2->radius) - dist;
        Vector2 separation = Vector2Scale(normal, overlap * 0.5f);
        crl1->centre = Vector2Subtract(crl1->centre, separation);
        crl2->centre = Vector2Add(crl2->centre, separation);
    }
}

void check_collision_w_circles(circle_array* arr, int i, Sound* s){
    double dist;
    for (int j = i; j < arr->len; j++)
    {
        if (i != j){
            dist = Vector2Distance((arr->circles[i].centre), arr->circles[j].centre);
            if (dist < (arr->circles[i].radius + arr->circles[j].radius)){
                set_new_speed_vector(&(arr->circles[i]), &(arr->circles[j]), dist);
                //if(Vector2Length(arr->circles[i].speed) > (SPEED / 2) || Vector2Length(arr->circles[j].speed) > (SPEED / 2))
                    //PlaySound(*s);
            }
        }   
    }
}

void change_selected_circle(circle_array* arr, circle** current_c_ptr, circle* main_c){
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 mouse_coords = {GetMouseX(), GetMouseY()};
        if (Vector2Distance(mouse_coords, main_c->centre) <= main_c->radius){
            if (*current_c_ptr != NULL){
                (*current_c_ptr)->color = MAIN_COLOR;
                (*current_c_ptr)->radius = RADIUS;
            }
            *current_c_ptr = main_c;
            (*current_c_ptr)->color = SELECTED_CIRCLE_COLOR;
            (*current_c_ptr)->radius = SELECTED_CIRCLE_RADIUS;
            return;
        }
        for (int i = 0; i < arr->len; i++)
        {
            if (Vector2Distance(mouse_coords, arr->circles[i].centre) <= arr->circles[i].radius){
                if (*current_c_ptr != NULL){
                    (*current_c_ptr)->color = MAIN_COLOR;
                    (*current_c_ptr)->radius = RADIUS;
                }
                (*current_c_ptr) = &(arr->circles[i]);
                (*current_c_ptr)->color = SELECTED_CIRCLE_COLOR;
                (*current_c_ptr)->radius = SELECTED_CIRCLE_RADIUS;
                return;
            }
        }
    }
    if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT)))
        if (*current_c_ptr != NULL){
            (*current_c_ptr)->color = MAIN_COLOR;
            (*current_c_ptr)->radius = RADIUS;
            (*current_c_ptr)->speed.x = 0;
            (*current_c_ptr)->speed.y = 0;
        }
        *current_c_ptr = NULL;  
}

void create_chaos(circle* c){
    c->speed.x = (rand() % MAX_RANDOM_SPEED - MAX_RANDOM_SPEED / 2);
    c->speed.y = (rand() % MAX_RANDOM_SPEED - MAX_RANDOM_SPEED / 2);
}

void handle_chaos(circle_array* arr)
{
    if (IsKeyPressed(KEY_ENTER)){
        for (int i = 0; i < arr->len; i++)
            create_chaos(&(arr->circles[i]));
    }
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "raylib!");
    InitAudioDevice();
    SetTargetFPS(70);

    Sound collision_sound = LoadSound("resources/sound_collision.wav");

    Vector2 c1Position = {200, 200};
    circle main_c = {c1Position, RADIUS, MAIN_COLOR};

    circle* selected_circe = &main_c;

    circle_array arr;
    arr.len = 0;
    arr.capacity = START_CAP;
    arr.circles = malloc(sizeof(circle) * arr.capacity);

    while (!WindowShouldClose())
    {
        handle_add_circle(&arr, RADIUS, MAIN_COLOR);
        handle_move(selected_circe);
        handle_chaos(&arr);
        change_selected_circle(&arr, &selected_circe, &main_c);
        handle_physic(&arr, selected_circe, &collision_sound);
        
        BeginDrawing();
        {
            ClearBackground(BACKCOLOR);
            DrawText("BACKSPACE TO DELETE ALL", 10, 10, 20, GRAY);
            DrawText("SPACE TO CREATE", 10, 50, 20, GRAY);
            DrawText("ENTER TO CHAOS", 10, 90, 20, GRAY);
            DrawText("DRAG & DROP", 10, 130, 20, GRAY);
            draw_rays(main_c.centre, RAYS_AMOUNT, &arr, MAIN_COLOR);
            DrawCircleV(main_c.centre, main_c.radius, main_c.color);
            for (int i = 0; i < arr.len; i++)
                DrawCircleV(arr.circles[i].centre, arr.circles[i].radius, arr.circles[i].color);
        }
        EndDrawing();  
        free_circles_if_pressed(&arr);
    }
    UnloadSound(collision_sound);

    CloseAudioDevice();

    CloseWindow();

    return 0;
}