// file: bridge.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// middleman between CORE and VIEW

let BRDG =
{

  // Initialise everything.
  // - Entry point for View (ie. HTML).
  init: function ()
  {
    // Initialise world
    let c = VIEW.get_canvas();
    WORLD.canvas = c.canvas;
    WORLD.c = c.context;
    BRDG.measure();

    // Initialise Model
    CORE.init();

    // Initialise View
    VIEW.init();
    VIEW.put_params(CORE.get_params());

    // Go!
    BRDG.resume();

    if (DEBUG) UTIL.debug_params(WORLD, STATE);
  },

  // Retrieve and world-set canvas dimensions.
  measure: function ()
  {
    let dim = VIEW.get_dimensions(WORLD.gap);
    WORLD.canvas.width = dim.w;
    WORLD.canvas.height = dim.h;
    WORLD.w = dim.w;
    WORLD.h = dim.h;
    WORLD.whalf = WORLD.w / 2;
    WORLD.hhalf = WORLD.h / 2;
  },

  // Handle canvas resize event.
  resize: function ()
  {
    BRDG.pause();
    BRDG.measure();
    BRDG.set_tick(0);
    VIEW.slider_set(0);
    VIEW.put_params(CORE.get_params());
    VIEW.put_start();
    CORE.init();
  },

  // Handle toggle (either pause or resume).
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
      if (WORLD.tick < WORLD.hbsz)
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
  tally: function () // TODO: testing & bug-bashing required
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

    let w = WORLD;
    let s = CORE.get_snapshot();
    let h = HIST;
    let k = w.tick;
    let z = w.hbsz;
    let tick = k % z;

    // Update by merging in buffer
    if (tick === 0)
    {
      // HTML slider is disabled by default and only gets enabled when
      // `tick` passes `hbsz`.
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
    let w = WORLD;
    let s = STATE;
    VIEW.paint(w.c, PTS, s.num, w.w, w.h, s.psz, BRDG.hue, w.crowd,
               w.theme);
  },

  // Given a particular neighborhood size, and a global average
  // neighborhood size, determine the color.
  // - Caching is used for performance optimisation.
  hue: function (n, avg, theme)
  {
    let h = WORLD.hues;

    if (h[n] === undefined)
    {
      let c;
      if (n <= avg)
      {
        c = theme[3];
      }
      else if (n <= 2 * avg)
      {
        c = theme[4];
      }
      else if (n <= 4 * avg)
      {
        c = theme[5];
      }
      else
      {
        c = theme[6];
      }
      h[n] = c;
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

  // Hand over particles.
  get_pts: function ()
  {
    return PTS;
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

  // Set initial particle distribution scheme.
  set_distr: function (distr)
  {
    STATE.distr = distr;
    return distr;
  },

  // Set global tick (ie. frame of animation).
  set_tick: function (tick)
  {
    WORLD.tick = tick;
  },

  // Make View alert user regarding `stop`.
  put_stop_alert: function ()
  {
    VIEW.put_stop_alert();
  },

  clear_theme_cache: function ()
  {
    WORLD.hues = [];
  },

  // Given a partial or complete state object in abbreviated form,
  // set the global state.
  load: function (o)
  {
    let a = ABBREV;
    let s = STATE;

    // Set global state.
    UTIL.for_abbrev(a, function (key)
    {
      if (o[key])
      {
        s[a[key]] = o[key];
      }
    })

    // Optionally set radians squared
    if (o.r)
    {
      WORLD.radsq = o.r * o.r;
    }

    // Set particles
    let np = new Array(s.num);
    let nl;

    if (o.p)
    {
      // STATE.num has precedence over o.p.length
      let p;
      for (let i = 0; i < (o.p.length > s.num ? s.num : o.p.length);
           i++)
      {
        p = o.p[i];
        np[i] = new CORE.Pt(p.x, p.y, p.phi);
        np[i].n = 0;
        np[i].l = 0;
        np[i].r = 0;
        np[i].s = Math.sin(p.phi);
        np[i].c = Math.cos(p.phi);
      }
      nl = o.p.length;
    }

    else
    {
      for (let i = 0; i < s.num; i++)
      {
        np[i] = PTS[i];
      }
      nl = PTS.length;
    }

    // If new STATE.num is greater than either o.p.length (if provided)
    // or the previous PTS.length.
    for (let i = nl; i < s.num; i++)
    {
      np[i] = new CORE.Pt();
    }

    PTS = np;
  },
};

