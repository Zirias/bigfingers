#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

extern char binary_src_led_lit_bmp_size;
extern char binary_src_led_lit_bmp_start;
extern char binary_src_led_unlit_bmp_size;
extern char binary_src_led_unlit_bmp_start;

#define led_lit_bmp ((void *)&binary_src_led_lit_bmp_start)
#define led_lit_bmp_size ((size_t)&binary_src_led_lit_bmp_size)
#define led_unlit_bmp ((void *)&binary_src_led_unlit_bmp_start)
#define led_unlit_bmp_size ((size_t)&binary_src_led_unlit_bmp_size)

struct context
{
    SDL_Renderer *r;
    SDL_Texture *unlit;
    SDL_Texture *lit;
    int tx[5];
    int ty[5];
    int board[5][5];
};

void draw(const struct context *ctx)
{
    SDL_Rect dst;
    SDL_RenderClear(ctx->r);
    for (int r = 0; r < 5; ++r)
    {
        dst.y = r ? ctx->ty[r-1] : 0;
        dst.h = ctx->ty[r] - dst.y;
        for (int c = 0; c < 5; ++c)
        {
            dst.x = c ? ctx->tx[c-1] : 0;
            dst.w = ctx->tx[c] - dst.x;
            SDL_RenderCopy(ctx->r, ctx->board[r][c] ? ctx->lit : ctx->unlit,
                        0, &dst);
        }
    }
    SDL_RenderPresent(ctx->r);
}

void onResized(struct context *ctx, int width, int height)
{
    int sx = width / 5;
    int mx = width % 5;
    int sy = height / 5;
    int my = height % 5;
    int i;

    int x = 0;
    int y = 0;
    for (i = 0; i < 5; ++i)
    {
        x += sx;
        if (mx > (i^3)) ++x;
        ctx->tx[i] = x;
        y += sy;
        if (my > (i^3)) ++y;
        ctx->ty[i] = y;
    }

    draw(ctx);
}

void toggle(struct context *ctx, int r, int c)
{
    if (r > 0) ctx->board[r-1][c] = !ctx->board[r-1][c];
    if (r < 4) ctx->board[r+1][c] = !ctx->board[r+1][c];
    if (c > 0) ctx->board[r][c-1] = !ctx->board[r][c-1];
    if (c < 4) ctx->board[r][c+1] = !ctx->board[r][c+1];
    ctx->board[r][c] = !ctx->board[r][c];
}

void onClick(struct context *ctx, int x, int y)
{
    int r = 0;
    while (ctx->ty[r] < y) ++r;
    int c = 0;
    while (ctx->tx[c] < x) ++c;

    toggle(ctx, r, c);
    draw(ctx);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int rc = EXIT_SUCCESS;
    SDL_Window *w = 0;
    struct context ctx = {0,0,0,{50,100,150,200,250},{50,100,150,200,250},{{0}}};

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    w = SDL_CreateWindow("BigFingers",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                250, 250, SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    if (!w)
    {
        fprintf(stderr, "Error opening window: %s\n", SDL_GetError());
        goto error;
    }

    ctx.r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx.r)
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        goto error;
    }

    SDL_RWops *stream = SDL_RWFromConstMem(led_lit_bmp, led_lit_bmp_size);
    SDL_Surface *bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp) goto error;
    ctx.lit = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);
    stream = SDL_RWFromConstMem(led_unlit_bmp, led_unlit_bmp_size);
    bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp) goto error;
    ctx.unlit = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);

    srand(time(0));
    for (int i = 0; i < 128; ++i)
    {
        toggle(&ctx, rand()%5, rand()%5);
    }

    draw(&ctx);
    SDL_Event ev;
    while (SDL_WaitEvent(&ev))
    {
        if (ev.type == SDL_QUIT) goto quit;
        switch (ev.type)
        {
        case SDL_QUIT:
            goto quit;
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                onResized(&ctx, ev.window.data1, ev.window.data2);
            }
        case SDL_MOUSEBUTTONDOWN:
            if (ev.button.button == SDL_BUTTON_LEFT)
            {
                onClick(&ctx, ev.button.x, ev.button.y);
            }
        }
    }

error:
    rc = EXIT_FAILURE;
quit:
    if (ctx.r) SDL_DestroyRenderer(ctx.r);
    if (w) SDL_DestroyWindow(w);
    SDL_Quit();
    return rc;
}
