// file: util.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// utility

let U =
{

  is_empty: function (o)
  {
    return Object.keys(o).length === 0 && o.constructor === Object;
  },

  mod: function (n, lim)
  {
    n %= lim;
    if (n < 0)
    {
      n += lim;
    }
    return n;
  },

  rad: function (n)
  {
    return n / 360 * TAU;
  },

  deg: function (n)
  {
    return n * 360 / TAU;
  },

  prep: function (state)
  {
    state.a = U.rad(state.a);
    state.b = U.rad(state.b);
  },

  unprep: function (state)
  {
    state.a = U.deg(state.a);
    state.b = U.deg(state.b);
  },

  debug_params: function (world, state)
  {
    console.log("EMERGENCE\n---------" +
                "\nfps   = " + world.fps +
                "\nnum   = " + state.n +
                "\ndens  = " + state.d +
                "\nalpha = " + U.deg(state.a) +
                "\nbeta  = " + U.deg(state.b) +
                "\ngamma = " + state.g +
                "\nrad   = " + state.r +
                "\nspd   = " + state.s +
                "\n---------");
  },
};
