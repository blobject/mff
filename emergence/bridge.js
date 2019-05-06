// file: bridge.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// connect core (E) to draw (D)

let B =
{

  init: function ()
  {
    // initialise draw
    D.init();
    let c = D.get_canvas();
    W.canvas = c.canvas;
    W.c = c.context;
    B.measure();

    // initialise core
    E.init();
    U.prep(S);
    B.resume();

    if (DEBUG) U.debug_params(W, S);
  },

  measure: function ()
  {
    let dim = D.get_dimensions(W.gap);
    W.canvas.width = dim.w;
    W.canvas.height = dim.h;
    W.w = dim.w;
    W.h = dim.h;
    W.w2 = W.w / 2;
    W.h2 = W.h / 2;
  },

  resize: function ()
  {
    B.measure();
    E.init();
    B.pause();
    D.say_restart();
  },

  resume: function ()
  {
    if (W.paused)
    {
      B.toggle();
    }
  },

  pause: function ()
  {
    if (!W.paused)
    {
      B.toggle();
    }
  },

  toggle: function ()
  {
    D.toggle(W.paused, E.tick, W.fps);
    if (W.paused)
    {
      W.paused = false;
    }
    else
    {
      W.paused = true;
    }
  },

  paint: function ()
  {
    let w = W;
    let s = S;
    D.paint(w.c, P, s.n, w.w, w.h, s.z, B.hue, s.x, w.t[0]);
  },

  hue: function (n, avg, theme)
  {
    let h = W.x;
    if (h[n] === undefined)
    {
      let c;
      if (n <= avg)
      {
        c = theme[2];
      }
      else if (n <= 2 * avg)
      {
        c = theme[3];
      }
      else if (n <= 5 * avg)
      {
        c = theme[4];
      }
      else
      {
        c = theme[5];
      }
      h[n] = c;
    }

    return h[n];
  },

  get_world: function ()
  {
    return W;
  },

  get_state: function ()
  {
    return S;
  },

  get_pts: function ()
  {
    return P;
  },

  get_save: function ()
  {
    let s = JSON.parse(JSON.stringify(S)); // clone
    U.unprep(s);
    s.p = [];
    let p;
    for (let i = 0; i < s.n; i++)
    {
      p = P[i];
      s.p[i] =
      {
        n: i + 1,
        x: p.x,
        y: p.y,
        f: p.f,
      };
    }
    return s;
  },

  load: function (o)
  {
    U.unprep(S);
    if (o.f) W.fps = o.f;
    if (o.n) S.n = o.n;
    if (o.z) S.z = o.z;
    if (o.d) S.d = o.d;
    if (o.a) S.a = o.a;
    if (o.b) S.b = o.b;
    if (o.g) S.g = o.g;
    if (o.p)
    {
      // S.n has precedence over o.p.length
      let np = new Array(S.n);
      let p;
      for (let i = 0; i < (o.p.length > S.n ? S.n : o.p.length); i++)
      {
        p = o.p[i];
        np[i] = new E.Pt(p.x, p.y, p.f);
        np[i].n = 0;
        np[i].l = 0;
        np[i].r = 0;
        np[i].s = SIN(p.f);
        np[i].c = COS(p.f);
      }
      for (let i = o.p.length; i < S.n; i++)
      {
        np[i] = new E.Pt();
      }
      P = np;
    }
  },
};
