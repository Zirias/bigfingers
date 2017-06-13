#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "resource.h"

DECLRES(src_led_lit_bmp);
DECLRES(src_led_unlit_bmp);
DECLRES(src_finger_bmp);

enum fingerstate
{
    FS_HIDDEN,
    FS_BLEND,
    FS_SOLID
};

struct context
{
    SDL_Window *w;
    SDL_Renderer *r;
    SDL_Texture *unlit;
    SDL_Texture *lit;
    SDL_Texture *finger;
    int tx[5];
    int ty[5];
    int board[5][5];
    int moves;
    enum fingerstate fingerstate;
    int fingerpos[2];
};

static void draw(const struct context *ctx)
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
    if (ctx->fingerstate != FS_HIDDEN)
    {
        if (ctx->fingerstate == FS_BLEND)
        {
            SDL_SetTextureAlphaMod(ctx->finger, 0xc0);
        }
        else
        {
            SDL_SetTextureAlphaMod(ctx->finger, 0xff);
        }
        dst.w = ctx->tx[0] * 2;
        dst.h = ctx->tx[0] * 2;
        dst.x = ctx->fingerpos[0];
        dst.y = ctx->fingerpos[1];
        SDL_RenderCopy(ctx->r, ctx->finger, 0, &dst);
    }
    SDL_RenderPresent(ctx->r);
}

static void onResized(struct context *ctx, int width, int height)
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
}

static void onMouseMove(struct context *ctx, int x, int y)
{
    int ox = 6 * ctx->tx[0] / 5;
    int oy = 6 * ctx->ty[0] / 5;

    int c = 0;
    while (ctx->tx[c] < x) ++c;
    ox += (ctx->tx[c] - x) / 3;
    int r = 0;
    while (ctx->ty[r] < y) ++r;
    oy += (ctx->ty[r] - y) / 3;

    ctx->fingerpos[0] = ctx->tx[c] - ox;
    ctx->fingerpos[1] = ctx->ty[r] - oy;
}

static void toggle(struct context *ctx, int r, int c)
{
    if (r > 0) ctx->board[r-1][c] = !ctx->board[r-1][c];
    if (r < 4) ctx->board[r+1][c] = !ctx->board[r+1][c];
    if (c > 0) ctx->board[r][c-1] = !ctx->board[r][c-1];
    if (c < 4) ctx->board[r][c+1] = !ctx->board[r][c+1];
    ctx->board[r][c] = !ctx->board[r][c];
}

static int onClick(struct context *ctx, int x, int y)
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
    struct context ctx = {0, 0, 0, 0, 0,
	{50,100,150,200,250},
	{50,100,150,200,250}, {{0}}, 0, 0, {0}};

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

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    ctx.r = SDL_CreateRenderer(ctx.w, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx.r)
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        goto error;
    }

    SDL_RWops *stream = SDL_RWFromConstMem(RESOURCE(src_led_lit_bmp),
	    RESSIZE(src_led_lit_bmp));
    SDL_Surface *bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp) goto error;
    ctx.lit = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);
    stream = SDL_RWFromConstMem(RESOURCE(src_led_unlit_bmp),
	    RESSIZE(src_led_unlit_bmp));
    bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp) goto error;
    ctx.unlit = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);
    stream = SDL_RWFromConstMem(RESOURCE(src_finger_bmp),
            RESSIZE(src_finger_bmp));
    bmp = SDL_LoadBMP_RW(stream, 1);
    if (!bmp) goto error;
    SDL_SetColorKey(bmp, SDL_TRUE, 0xff00);
    ctx.finger = SDL_CreateTextureFromSurface(ctx.r, bmp);
    SDL_FreeSurface(bmp);
    SDL_SetTextureBlendMode(ctx.finger, SDL_BLENDMODE_BLEND);

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
	    else if (ev.window.event == SDL_WINDOWEVENT_EXPOSED)
	    {
		draw(&ctx);
	    }
            else if (ev.window.event == SDL_WINDOWEVENT_LEAVE)
            {
                ctx.fingerstate = FS_HIDDEN;
                draw(&ctx);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (ev.button.button == SDL_BUTTON_LEFT)
            {
                ctx.fingerstate = FS_SOLID;
		if (onClick(&ctx, ev.button.x, ev.button.y))
		{
		    goto quit;
		}
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev.button.button == SDL_BUTTON_LEFT)
            {
                ctx.fingerstate = FS_BLEND;
                draw(&ctx);
            }
            break;
        case SDL_MOUSEMOTION:
            ctx.fingerstate = FS_BLEND;
            onMouseMove(&ctx, ev.motion.x, ev.motion.y);
            draw(&ctx);
            break;
        }
    }

error:
    rc = EXIT_FAILURE;
quit:
    if (ctx.finger) SDL_DestroyTexture(ctx.finger);
    if (ctx.unlit) SDL_DestroyTexture(ctx.unlit);
    if (ctx.lit) SDL_DestroyTexture(ctx.lit);
    if (ctx.r) SDL_DestroyRenderer(ctx.r);
    if (ctx.w) SDL_DestroyWindow(ctx.w);
    SDL_Quit();
    return rc;
}
