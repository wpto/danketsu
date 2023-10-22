#include "test_renderer.h"

#define CURSOR_TEXTURE_ID 16

#define GRASS_TILE_ID 1
#define ROAD_TILE_ID 4
#define WATER_TILE_ID 5

static bool app_init(Scene *app) {
  RenderExInit(&app->r);
  RenderTextureLoad(&app->textures[0], "assets/sprite1.png");
  // jsmn_parser parser;
  // jsmntok_t tokens[10];

  // jsmn_init(&parser);
  // jsmn_parse(&parser, jsonString, jsonStringLen, tokens, 10)

  u32 tileWidth = 64;
  u32 tileHeight = 64;
  RenderTextureLoad(&app->textures[1], "assets/tower1.png");

  TileMapLoadCSV(&app->background, "assets/map1_background.csv",
                 &app->textures[1], tileWidth, tileHeight);
  TileMapLoadCSV(&app->shadows, "assets/map1_shadows.csv", &app->textures[1],
                 tileWidth, tileHeight);

  RenderTextureLoad(&app->textures[2], "assets/NSimSun1.bmp");
  TileMapInit(&app->font, &app->textures[2], 14, 32, 32, 32);

  PathPoint *path = app->path;
  path[0].position[0] = 3.0f;
  path[0].position[1] = 2.0f;
  path[1].position[0] = 3.0f;
  path[1].position[1] = 12.0f;
  path[2].position[0] = 4.0f;
  path[2].position[1] = 13.0f;
  path[3].position[0] = 6.0f;
  path[3].position[1] = 13.0f;
  path[4].position[0] = 7.0f;
  path[4].position[1] = 12.0f;
  path[5].position[0] = 7.0f;
  path[5].position[1] = 4.0f;
  path[6].position[0] = 8.0f;
  path[6].position[1] = 3.0f;
  path[7].position[0] = 12.0f;
  path[7].position[1] = 3.0f;
  path[8].position[0] = 13.0f;
  path[8].position[1] = 4.0f;
  path[9].position[0] = 13.0f;
  path[9].position[1] = 7.0f;
  path[10].position[0] = 11.0f;
  path[10].position[1] = 9.0f;
  path[11].position[0] = 11.0f;
  path[11].position[1] = 12.0f;
  path[12].position[0] = 12.0f;
  path[12].position[1] = 13.0f;
  path[13].position[0] = 15.0f;
  path[13].position[1] = 13.0f;

  app->pathLen = 14;

  for (u32 i = 0; i < app->pathLen; i++) {
    path[i].position[0] *= tileWidth;
    path[i].position[0] -= tileWidth / 2;
    path[i].position[1] *= tileHeight;
    path[i].position[1] -= tileHeight / 2;
  }

  app->enemySlimeCap = 64;
  app->lastSpawnTime = 0;

  app->projectileCap = 48;
  app->projectileIdx = 0;

  return true;
}

static void app_update(Scene *app, float delta) {
  u32 nowTicks = SDL_GetTicks();

  f32 tileWidth = 64.0f;
  f32 tileHeight = 64.0f;

  // update
  i32 mousePosition[2];
  u32 mouseState = SDL_GetMouseState(&mousePosition[0], &mousePosition[1]);

  b8 mouseLeftPrev = app->mouseLeftPrev;
  b8 mouseLeftActive = !!(SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState);

  f32 cursorPosition[4] = {0.0f, 0.0f, tileWidth, tileHeight};
  cursorPosition[0] = (i32)(mousePosition[0] / tileWidth) * tileWidth;
  cursorPosition[1] = (i32)(mousePosition[1] / tileHeight) * tileHeight;

  f32 cursorUV[4] = {0.0f};
  TileMapUVByIdx(cursorUV, &app->background, CURSOR_TEXTURE_ID);

  b8 hoveringTower = 0;
  u32 hoveringTowerIndex = 0;
  // NOTE: cursor is red when there is already a tower.
  for (u32 i = 0; i < app->towersIdx; i++) {
    Tower *t = &app->towers[i];
    if (t->enable && f32Equal(t->position[0], cursorPosition[0]) &&
        f32Equal(t->position[1], cursorPosition[1])) {
      hoveringTower = 1;
      hoveringTowerIndex = i;
      // cursorColor = colorRed;
      break;
    }
  }

  b8 isValidFloorForTower = 1;
  i32 tile = TileMapTileByPosition(&app->background, cursorPosition);
  if (tile == WATER_TILE_ID || tile == ROAD_TILE_ID) {
    isValidFloorForTower = 0;
  }

  if (!hoveringTower && isValidFloorForTower && mouseLeftActive &&
      !mouseLeftPrev) {

    // TODO: how to deal with static array if fully used
    Tower *t = &app->towers[app->towersIdx];
    towerNew(t, cursorPosition);
    app->towersIdx++;
  }

  if (nowTicks - app->lastSpawnTime > 1000 &&
      app->enemySlimeLen < app->enemySlimeCap) {
    u32 i = app->enemySlimeLen;

    app->enemySlime[i].enable = 1;
    v4Set(app->enemySlime[i].position, 0.0f, 0.0f, 64.0f, 64.0f);
    app->enemySlime[i].bodyTextureIndex = 17;
    app->enemySlime[i].eyeTextureIndex = 18;
    app->enemySlime[i].speed = 20.0f;
    app->enemySlime[i].pathPointToFollow = 0;

    app->enemySlimeLen++;
    app->lastSpawnTime = SDL_GetTicks();
  }

  for (u32 i = 0; i < app->enemySlimeLen; i++) {
    EnemySlime *e = &app->enemySlime[i];
    PathPoint *p = &app->path[e->pathPointToFollow];

    f32 velocity[2] = {p->position[0], p->position[1]};

    v2Subtract(velocity, e->position);

    f32 dist = v2DistanceSquare(velocity);
    LogInfo("distance: %f", dist);
    if (dist < 100.0f && e->pathPointToFollow + 1 < app->pathLen) {
      e->pathPointToFollow += 1;
    }

    v2Normalize(velocity);
    v2Mult(velocity, e->speed * delta);
    v2Add((v2)&e->position, velocity);
  }

  b8 cursorShown = isValidFloorForTower;
  f32 *cursorColor = colorWhite;
  if (hoveringTower) {
    cursorColor = colorRed;
  }

  // NOTE: moving projectiles toward enemies.
  for (u32 i = 0; i < app->projectileIdx; i++) {
    Projectile *pr = &app->proj[i];

    // NOTE: projectile should be alive.
    if (!pr->enable) {
      continue;
    }

    EnemySlime *e = &app->enemySlime[pr->entityTarget];

    if (e->enable) {
      v2Copy(pr->lastTargetKnownPosition, e->position);
    }

    f32 velocity[2];
    v2Copy(velocity, pr->lastTargetKnownPosition);
    v2Subtract(velocity, pr->position);
    v2Normalize(velocity);
    v2Mult(velocity, pr->speed * delta);

    if (v2DistanceSquare2(pr->lastTargetKnownPosition, pr->position) <
        pr->speed * pr->speed) {
      // TODO: collide check success. apply damage to the enemy.
      pr->enable = 0;
    }

    v2Add(pr->position, velocity);
  }

  for (u32 i = 0; i < app->towersIdx; i++) {
    Tower *t = &app->towers[i];

    if (nowTicks > t->lastShootTick + t->shootCooldown) {
      b8 enemyFound = 0;
      u32 enemyIndex = 0;

      // NOTE: looking for enemy
      for (u32 e = 0; e < app->enemySlimeLen; e++) {
        if (!app->enemySlime[e].enable) {
          continue;
        }

        f32 distance =
            v2DistanceSquare2(app->enemySlime[e].position, t->position);

        if (distance < t->detectRangeSquared) {
          enemyFound = 1;
          enemyIndex = e;
          break;
        }
      }

      if (enemyFound && app->projectileIdx < app->projectileCap) {
        // NOTE: Find empty projectile slot.

        b8 emptyProjectileIndexFound = 0;
        u32 emptyProjectileIndex = 0;

        for (u32 offset = 1; offset < app->projectileCap; offset++) {
          u32 testIndex =
              app->lastCreatedProjectileIndex + offset % app->projectileCap;

          if (app->proj[testIndex].enable) {
            continue;
          }

          break;
        }
        u32 pi = app->projectileIdx;
        Projectile *p = &app->proj[pi];

        p->enable = 1;
        p->position[0] = t->position[0];
        p->position[1] = t->position[1];
        p->position[2] = 16;
        p->position[3] = 16;

        p->speed = 50;

        p->entityTarget = enemyIndex;
        v2Copy(p->lastTargetKnownPosition,
               app->enemySlime[enemyIndex].position);

        app->projectileIdx++;

        t->lastShootTick = nowTicks;
      }
    }
  }

  // update defer

  app->mouseLeftPrev = mouseLeftActive;

  // draw

  Texture *cursorTexture = &app->textures[1];

  RendererEx *rx = &app->r;
  Renderer *r = &rx->r;
  RenderBeginFrame(r);

  u32 spriteWidth = 113;
  u32 spriteHeight = 128;

  // float a[2] = {0, 0};
  // float b[2] = {(f32)(spriteWidth), 0};
  // float c[2] = {0, (f32)(spriteHeight)};
  // float ac[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  // float bc[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  // float cc[4] = {0.0f, 0.0f, 1.0f, 1.0f};
  // float aUV[2] = {0.0f, 0.0f};
  // float bUV[2] = {(f32)(spriteWidth) / (app->textures[0].width), 0.0f};
  // float cUV[2] = {0.0f, (f32)(spriteHeight) / (app->textures[0].height)};
  // u32 texture = app->textures[0].texture;

  // RenderPushTriangle(r, a, b, c, ac, bc, cc, aUV, bUV, cUV, texture);

  // f32 pos[4] = {750.0f, 750.0f, 100.0f, 100.0f};
  // // f32 col[4] = {1.0f, 0.5f, 0.5f, 1.0f};

  // // RenderPushQuadColor(rx, pos, col);

  // RenderPushQuadTex(rx, pos, app->textures[0].texture, white);

  // f32 posx[4] = {100.0f, 100.0f, (f32)spriteWidth, (f32)spriteHeight};
  // f32 uvx[4] = {0.0f, 0.0f, (f32)(spriteWidth) / (app->textures[0].width),
  //               (f32)(spriteHeight) / (app->textures[0].height)};
  // RenderPushQuadSubTex(rx, posx, uvx, app->textures[0].texture, white);

  DrawTileMapLayer(rx, &app->background);
  DrawTileMapLayer(rx, &app->shadows);

  Texture *towerTexture = &app->textures[1];
  Texture *hoverRangeTexture = towerTexture;

  if (hoveringTower) {
    Tower *t = &app->towers[hoveringTowerIndex];

    f32 range = (f32)(t->detectRange) / 2;
    f32 leftX = (f32)(t->position[0] + t->spawnPosition[0]) - range;
    f32 topY = (f32)(t->position[1] + t->spawnPosition[1]) - range;
    f32 rangePosition[4] = {leftX, topY, range, range};

    f32 uv[4] = {0.0f};
    TileMapUVByIdx(uv, &app->background, 20);

    RenderPushQuadSubTex(rx, rangePosition, uv, hoverRangeTexture->texture,
                         colorWhite);

    // NOTE: top right tile
    rangePosition[0] = leftX + range;
    TileMapUVByIdx(uv, &app->background, 21);
    RenderPushQuadSubTex(rx, rangePosition, uv, hoverRangeTexture->texture,
                         colorWhite);

    // NOTE: bottom left tile
    rangePosition[0] = leftX;
    rangePosition[1] = topY + range;
    TileMapUVByIdx(uv, &app->background, 22);
    RenderPushQuadSubTex(rx, rangePosition, uv, hoverRangeTexture->texture,
                         colorWhite);

    // NOTE: bottom right tile
    rangePosition[0] = leftX + range;
    rangePosition[1] = topY + range;
    TileMapUVByIdx(uv, &app->background, 23);
    RenderPushQuadSubTex(rx, rangePosition, uv, hoverRangeTexture->texture,
                         colorWhite);
  }

  // draw cursor
  if (cursorShown) {
    RenderPushQuadSubTex(rx, cursorPosition, cursorUV, cursorTexture->texture,
                         cursorColor);
  }

  for (u32 i = 0; i < ArrayCount(app->towers); i++) {
    Tower *t = &app->towers[i];
    if (!t->enable)
      continue;

    f32 uv[4] = {0.0f};
    TileMapUVByIdx(uv, &app->background, t->tileIdx);

    RenderPushQuadSubTex(rx, t->position, uv, towerTexture->texture,
                         colorWhite);
  }

  for (u32 j = 0; j < app->enemySlimeLen; j++) {
    u32 i = app->enemySlimeLen - 1 - j;
    EnemySlime *e = &app->enemySlime[i];
    if (!e->enable) {
      continue;
    }
    f32 bodyUV[4] = {0.0f};
    TileMapUVByIdx(bodyUV, &app->background, e->bodyTextureIndex);
    RenderPushQuadSubTex(rx, e->position, bodyUV, cursorTexture->texture,
                         colorRed);

    f32 eyeUV[4] = {0.0f};
    TileMapUVByIdx(eyeUV, &app->background, e->eyeTextureIndex);
    RenderPushQuadSubTex(rx, e->position, eyeUV, cursorTexture->texture,
                         colorWhite);
  }

  for (u32 i = 0; i < app->projectileIdx; i++) {
    Projectile *pr = &app->proj[i];

    if (!pr->enable) {
      continue;
    }

    RenderPushQuadColor(rx, pr->position, colorWhite);
  }

  f32 textPos[4] = {0.0f, 0.0f, 100.0f, 32.0f};
  char buf[20];
  sprintf_s(buf, "pos: %.2f %.2f", app->enemySlime[0].position[0],
            app->enemySlime[0].position[1]);
  RenderPushString(rx, textPos, buf, &app->font, colorWhite);

  RenderEndFrame(r);
}

static void app_input(Scene *app, SDL_Event e) {
  // switch (e.type) {
  // case SDL_MOUSEMOTION:
  // i32 x, y;
  // SDL_GetMouseState(&x, &y);
  // LogInfo("event=(%d, %d) state=(%d, %d)", e.motion.x, e.motion.y, x, y);
  // }
}
static void AppClean(Scene *app) { RenderFree(&app->r.r); }

static void DrawTileMapLayer(RendererEx *rx, TileMap *tm) {
  f32 tileWidth = (f32)(tm->tileWidth);
  f32 tileHeight = (f32)(tm->tileHeight);

  for (u32 row = 0; row < tm->height; row++) {
    for (u32 col = 0; col < tm->width; col++) {
      i32 tile = tm->map[row][col];
      if (tile == -1) {
        continue;
      }

      f32 tilePosition[4] = {(f32)(col * tileWidth), (f32)(row * tileHeight),
                             tileWidth, tileHeight};

      f32 tileUV[4] = {0.0f};
      TileMapUVByIdx(tileUV, tm, tile);
      RenderPushQuadSubTex(rx, tilePosition, tileUV, tm->texture->texture,
                           colorWhite);
    }
  }
}
