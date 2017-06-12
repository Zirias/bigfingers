#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "resource.h"

DECLRES(src_led_lit_bmp);
DECLRES(src_led_unlit_bmp);

struct context
{
    SDL_Window *w;
    SDL_Renderer *r;
    SDL_Texture *unlit;
    SDL_Texture *lit;
    int tx[5];
    int ty[5];
    int board[5][5];
    int moves;
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

int onClick(struct context *ctx, int x, int y)
{
    int r = 0;
    while (ctx->ty[r] < y) ++r;
    int c = 0;
    while (ctx->tx[c] < x) ++c;

    toggle(ctx, r, c);
    ++ctx->moves;

    int won = 1;
    for (int i = 0; i < 25; ++i)
    {
	if (((int *)ctx->board)[i])
	{
	    won = 0;
	    break;
	}
    }

    draw(ctx);

    if (won)
    {
	const SDL_MessageBoxButtonData buttons[] = {
	    { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "yes" },
	    { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "no" }
	};
	char message[128];
	snprintf(message, 128, "You won in %d moves.\n\n"
		"Do you want to play again?", ctx->moves);
	const SDL_MessageBoxData mbox = {SDL_MESSAGEBOX_INFORMATION,
	    ctx->w, "You won!", message, SDL_arraysize(buttons), buttons, 0};
	int button;
	SDL_ShowMessageBox(&mbox, &button);
	if (button)
	{
	    return 1;
	}
	else
	{
	    for (int i = 0; i < 128; ++i)
	    {
		toggle(ctx, rand()%5, rand()%5);
	    }
	    ctx->moves = 0;
	    draw(ctx);
	    return 0;
	}
    }

    return 0;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int rc = EXIT_SUCCESS;
    struct context ctx = {0, 0, 0, 0,
	{50,100,150,200,250},
	{50,100,150,200,250}, {{0}}, 0};

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    ctx.w = SDL_CreateWindow("BigFingers",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                250, 250, SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    if (!ctx.w)
    {
        fprintf(stderr, "Error opening window: %s\n", SDL_GetError());
        goto error;
    }

    ctx.r = SDL_CreateRenderer(ctx.w, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx.r)
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        goto error;
    }

    SDL_RWops *stream = SDL_RWFromConstMem(RESOURCE(src_led_lit_bmp),
	    RESSIZE(src_led_lit_bmp));
    SDL_Surface *bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp)
    {
        fprintf(stderr, "Error creating surface: %s\n", SDL_GetError());
	goto error;
    }
    ctx.lit = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);
    stream = SDL_RWFromConstMem(RESOURCE(src_led_unlit_bmp),
	    RESSIZE(src_led_unlit_bmp));
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
		if (onClick(&ctx, ev.button.x, ev.button.y))
		{
		    goto quit;
		}
            }
        }
    }

error:
    rc = EXIT_FAILURE;
quit:
    if (ctx.unlit) SDL_DestroyTexture(ctx.unlit);
    if (ctx.lit) SDL_DestroyTexture(ctx.lit);
    if (ctx.r) SDL_DestroyRenderer(ctx.r);
    if (ctx.w) SDL_DestroyWindow(ctx.w);
    SDL_Quit();
    return rc;
}
