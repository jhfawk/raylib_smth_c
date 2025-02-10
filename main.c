#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 450
#define SPEED 2.0f
#define START_CAP 5 
#define RAYS_AMOUNT 60
#define RADIUS 25
#define SELECTED_CIRCLE_COLOR RED

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
void check_collision_w_border(circle* c);
void handle_physic(circle_array* arr, circle* selected_circle);
void handle_position(circle_array* arr);
void set_new_speed_vector(circle* crl1, circle* crl2);
void check_collision_w_circles(circle_array* arr, int i);
void change_selected_circle(circle_array* arr, circle** current_c_ptr, circle* main_c);
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
    if (IsKeyPressed(KEY_ENTER)){
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
    double stop = ((HEIGHT > WIDTH) ? HEIGHT : WIDTH);
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

void check_collision_w_border(circle* c){
    if ((c->centre.x + c->radius) > WIDTH || (c->centre.x - c->radius) < 0){
        c->speed.x *= (-1 * 0.7);
    }
    if ((c->centre.y + c->radius) > HEIGHT || (c->centre.y - c->radius) < 0){
        c->speed.y *= (-1 * 0.7);
    }
}

void handle_physic(circle_array* arr, circle* selected_circle){
    for (int i = 0; i < arr->len; i++)
    {
        if(&(arr->circles[i]) != selected_circle){
            check_collision_w_circles(arr, i); 
            arr->circles[i].speed = Vector2Add(arr->circles[i].speed, arr->circles[i].acceleration);
            arr->circles[i].centre = Vector2Add(arr->circles[i].speed, arr->circles[i].centre);
            check_collision_w_border(&(arr->circles[i]));
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

void set_new_speed_vector(circle* crl1, circle* crl2){
    float overlap;
    double dist = Vector2Distance(crl1->centre, crl2->centre);
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

void check_collision_w_circles(circle_array* arr, int i){
    for (int j = i; j < arr->len; j++)
    {
        if (i != j)
            set_new_speed_vector(&(arr->circles[i]), &(arr->circles[j]));
    }
}

void change_selected_circle(circle_array* arr, circle** current_c_ptr, circle* main_c){
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 mouse_coords = {GetMouseX(), GetMouseY()};
        if (Vector2Distance(mouse_coords, main_c->centre) <= main_c->radius){
            if (*current_c_ptr != NULL)
                (*current_c_ptr)->color = RAYWHITE;
            *current_c_ptr = main_c;
            (*current_c_ptr)->color = SELECTED_CIRCLE_COLOR;
            return;
        }
        for (int i = 0; i < arr->len; i++)
        {
            if (Vector2Distance(mouse_coords, arr->circles[i].centre) <= arr->circles[i].radius){
                if (*current_c_ptr != NULL)
                    (*current_c_ptr)->color = RAYWHITE;
                (*current_c_ptr) = &(arr->circles[i]);
                (*current_c_ptr)->color = SELECTED_CIRCLE_COLOR;
                return;
            }
        }
    }
    if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT)))
        if (*current_c_ptr != NULL)
            (*current_c_ptr)->color = RAYWHITE;
        *current_c_ptr = NULL;  
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "raylib!");
    SetTargetFPS(70);

    Vector2 c1Position = {200, 200};
    circle main_c = {c1Position, RADIUS, RAYWHITE};

    circle* selected_circe = &main_c;

    circle_array arr;
    arr.len = 0;
    arr.capacity = START_CAP;
    arr.circles = malloc(sizeof(circle) * arr.capacity);

    while (!WindowShouldClose())
    {
        handle_add_circle(&arr, RADIUS, RAYWHITE);
        handle_move(selected_circe);
        change_selected_circle(&arr, &selected_circe, &main_c);
        handle_physic(&arr, selected_circe);
        
        BeginDrawing();
        {
            ClearBackground(BLACK);
            draw_rays(main_c.centre, RAYS_AMOUNT, &arr, RAYWHITE);
            DrawCircleV(main_c.centre, main_c.radius, main_c.color);
            for (int i = 0; i < arr.len; i++)
                DrawCircleV(arr.circles[i].centre, arr.circles[i].radius, arr.circles[i].color);

        }
        EndDrawing();  
        free_circles_if_pressed(&arr);
    }
    CloseWindow();

    return 0;
}