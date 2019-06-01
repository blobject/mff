// file: emergence/bridge.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// middleman between CORE and VIEW

const BRDG =
{

  // Initialise everything.
  // - Entry point for View (ie. HTML).
  init: function ()
  {
    // Initialise world
    Object.assign(WORLD, VIEW.get_canvas());
    BRDG.measure();

    // Initialise Model
    CORE.init(true); // fresh start

    // Initialise View
    VIEW.init();
    VIEW.put_params(CORE.get_params());

    // Go!
    BRDG.resume();

    if (DEBUG) { UTIL.debug_params(WORLD, STATE); }
  },

  // Retrieve and world-set canvas dimensions.
  measure: function ()
  {
    const dim = VIEW.get_dimensions();
    WORLD.canvas.width = dim.width;
    WORLD.canvas.height = dim.height;
    STATE.width = dim.width;
    STATE.height = dim.height;
    WORLD.width_half = STATE.width / 2;
    WORLD.height_half = STATE.height / 2;
  },

  // Handle canvas resize event.
  resize: function ()
  {
    BRDG.measure();
    VIEW.start(CORE.get_params(), true, true, true);
    VIEW.put_start();
  },

  // Handle toggle (ie. either pause or resume).
  toggle: function ()
  {
    VIEW.toggle(WORLD.paused, CORE.tick, STATE.fps);
    if (WORLD.paused)
    {
      WORLD.paused = false;
    }
    else
    {
      WORLD.paused = true;
    }
  },

  // Handle resume (unpause) event.
  resume: function ()
  {
    if (WORLD.paused)
    {
      if (WORLD.tick < WORLD.history_size)
      {
        VIEW.slider_disable();
      }
      BRDG.toggle();
    }
  },

  // Handle pause (unresume) event.
  pause: function ()
  {
    if (!WORLD.paused)
    {
      BRDG.toggle();
    }
  },

  // On every tick, register the state onto the history.
  tally: function ()
  {
    // SKETCH:
    //   Every k(=100) ticks, the buffer gets (lossily) squashed into
    //   the history proper, the latter of which is represented by the
    //   slider. This means that the second half of the slider is
    //   always very recent, whereas the first half gets more and more
    //   lossy and compressed.
    //
    // k=1         k=100       k=200
    //  .___________.___________.
    //        H0          B0    *0
    //
    //             k=200       k=300
    //  ._____._____.___________.
    //     H0    B0 *0          *1
    //        H1          B1
    //
    //             k=300       k=400
    //  .__.__._____.___________.
    //   H0 B0   B1 *1
    //        H2          B2

    const w = WORLD;
    const s = CORE.get_snapshot();
    const h = HIST;
    const k = w.tick;
    const z = w.history_size;
    const tick = k % z;

    // Update by merging in buffer
    if (tick === 0)
    {
      // HTML slider is disabled by default and only gets enabled when
      // `tick` passes `history_size`.
      if (k === z)
      {
        VIEW.slider_set(100);
        VIEW.slider_enable();
      }
      else
      {
        // Squash buffer into history proper
        for (let i = 0; i < z / 2; i++)
        {
          h.snaps[i] = h.snaps[i * 2];
        }
        for (let i = 0; i < z / 2; i++)
        {
          h.snaps[i + z / 2] = h.buf[i * 2];
        }
      }
      // Don't forget the last index which does not get squashed
      h.snaps[z - 1] = s;
    }

    // Populate history proper (when k < 100) then populate buffer (when
    // k = 100).
    else
    {
      if (k < z)
      {
        VIEW.slider_set(k);
        h.snaps[k - 1] = s;
      }
      else
      {
        h.buf[tick - 1] = s;
      }
    }
  },

  // Call View's drawing function.
  paint: function ()
  {
    const w = WORLD;
    const s = STATE;
    VIEW.paint(w.context, PTS, s.num, s.width, s.height, s.psize,
               BRDG.hue, s.density, w.theme);
  },

  // Given a particular neighborhood size, and a global average
  // neighborhood size, determine the color.
  // - Caching is used.
  hue: function (n, density, theme)
  {
    const h = WORLD.hues;

    if (UTIL.is_nil(h[n]))
    {
      let c = 0;
      if (theme === 0 || theme === 4)
      {
        c = 180;
      }
      else if (theme === 1)
      {
        c = 60;
      }
      else if (theme === 2)
      {
        c = 90;
      }
      else if (theme === 3)
      {
        c = 230;
      }
      h[n] = UTIL.mod((n * 60 / density) + c, 360);
    }

    return h[n];
  },

  // Hand over non-user-controllable portion of world state.
  get_world: function ()
  {
    return WORLD;
  },

  // Hand over user-controllable portion of world state.
  get_state: function ()
  {
    return STATE;
  },

  // Hand over default `STATE`.
  get_state_default: function ()
  {
    return STATED;
  },

  // Hand over history.
  get_history: function ()
  {
    return HIST;
  },

  // Hand over parameter name listing.
  get_abbrev: function ()
  {
    return ABBREV;
  },

  // Set alpha/beta configuration.
  get_config: function (c)
  {
    return CONFIGS[c];
  },

  // Set color theme.
  set_theme: function (theme)
  {
    WORLD.theme = COLORS[theme];
    return WORLD.theme;
  },

  // Set global tick (ie. frame of animation).
  set_tick: function (tick)
  {
    WORLD.tick = tick;
  },

  // Clear color theme's hue cache.
  clear_theme_cache: function ()
  {
    WORLD.hues = [];
  },

  // Alert user regarding `stop`.
  put_stop_alert: function ()
  {
    VIEW.put_stop_alert();
  },
};

