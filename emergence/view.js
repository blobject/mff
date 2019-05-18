// file: view.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// draw, using html + css + canvas + javascript

let VIEW =
{

  // Initialise View.
  // - Set event listeners and check certain browser function
  //   compatibilities.
  init: function ()
  {
    document.addEventListener("keyup", VIEW.handle_key);
    if (window.File && window.FileReader && window.FileList &&
        window.Blob)
    {
      document.getElementById("button-modal-open-real")
        .addEventListener("change", VIEW.handle_file);
      document.getElementById("button-modal-open")
        .classList.add("button-show");
    }
    document.getElementById("slider").oninput = VIEW.slider_load;
  },

  // Clear canvas.
  erase: function (c, w, h)
  {
    c.clearRect(0, 0, w, h);
  },

  // Draw particles on canvas.
  paint: function (c, pts, n, w, h, rad, hue, avg, theme)
  {
    let p;
    VIEW.erase(c, w, h);
    for (let i = 0; i < n; i++)
    {
      p = pts[i];
      c.fillStyle = "hsl(" + hue(p.n, avg, theme) + ",100%,50%)";
      c.fillRect(p.x, p.y, rad, rad);
    }
  },

  // Pause or resume animation.
  toggle: function (paused, tick, fps)
  {
    let button = document.getElementById("button-bar-toggle");
    if (paused)
    {
      button.value = "PAUSE";
      animate = setInterval(tick, (1000 / fps));
      if (WORLD.tick < WORLD.hbsz)
      {
        VIEW.slider_disable();
      }
      if (DEBUG) console.log("(toggle) play");
    }
    else
    {
      button.value = "RESUME";
      window.clearTimeout(animate);
      if (DEBUG) console.log("(toggle) pause");
    }
  },

  // Hand over HTML canvas object and its 2D context.
  get_canvas: function ()
  {
    let c = document.querySelector("canvas");
    return {
      canvas: c,
      context: c.getContext("2d"),
    };
  },

  // Hand over page dimensions.
  get_dimensions: function (gap)
  {
    return {
      w: window.innerWidth,
      h: window.innerHeight - gap,
    };
  },

  // Handle global key press event.
  handle_key: function (event)
  {
    switch (event.key)
    {
      case " ":
        BRDG.toggle();
        break;

      default:
        //if (DEBUG) console.log("(handle_key) unrecognised key");
    }
  },

  // Handle file open event.
  handle_file: function (event)
  {
    let r = new FileReader();
    r.onload = function()
    {
      let area = document.querySelector(".textarea.load");
      area.value = r.result;
      area.focus();
      event.target.value = null; // clear file input value
    };
    r.readAsText(event.target.files[0]);
  },

  // Handle "HELP" button on bar.
  button_bar_help: function ()
  {
    document.querySelector(".modal.help")
      .classList.toggle("modal-show");
    BRDG.pause();
  },

  // Handle "LOAD" button on bar.
  button_bar_load: function ()
  {
    let msg = VIEW.message_clear("load");
    let area = document.querySelector(".textarea.load");
    area.value = "";
    VIEW.modal_reset("load", function()
    {
      area.focus();
    });
    BRDG.pause();
  },

  // Handle "SAVE" button on bar.
  button_bar_save: function ()
  {
    let msg = VIEW.message_clear("save");
    let area = document.querySelector(".textarea.save");
    area.value = CORE.encode(CORE.get_snapshot());
    VIEW.modal_reset("save", function()
    {
      area.focus();
      area.select();
      area.scrollTop = 0;
    });
    BRDG.pause();
  },

  // Handle "RESTART" button on bar.
  button_bar_restart: function ()
  {
    let s = BRDG.get_state();
    let p = VIEW.get_params();
    BRDG.load(p);
    BRDG.toggle(); // to enforce fps
    BRDG.set_tick(0); // which also disables slider
    VIEW.put_stop_calm();
    VIEW.put_params(p);
    CORE.init();
    BRDG.resume();
    if (DEBUG) UTIL.debug_params(BRDG.get_world(), s);
  },

  // Handle "APPLY" button on bar.
  button_bar_apply: function()
  {
    let s = BRDG.get_state();
    let p = VIEW.get_params();
    BRDG.load(p);
    BRDG.toggle(); // to enforce fps
    VIEW.put_params(p);
    CORE.init(true, true); // keep particles and radius
    BRDG.resume();
    if (DEBUG) UTIL.debug_params(BRDG.get_world(), s);
  },

  // Handle "APPLY" button in Load modal.
  button_modal_apply: function ()
  {
    let msg = VIEW.message_clear("load");
    let text = document.querySelector(".textarea.load").value;
    let s = BRDG.get_state();
    let parsed;
    try
    {
      //parsed = BRDG.load_raw(text);
      parsed = CODE.decode(text);
      if (UTIL.is_empty(parsed))
      {
        throw "empty state";
      }
      VIEW.load(parsed, true); // keep old parameters
    }
    catch(err)
    {
      msg.classList.add("bad");
      msg.textContent = 'Unparsable (see "HELP" on state)';
      if (DEBUG) console.log(err);
      if (DEBUG) console.log("(button_modal_apply) parse & load failed");
      return;
    }
    VIEW.button_bar_load(); // close modal
    CORE.init(true, true); // keep particles and radius
    if (parsed.p)
    {
      BRDG.set_tick(0); // which also disables slider
    }
    BRDG.resume();
    if (DEBUG) console.log("(button_modal_apply) applied");
    if (DEBUG) UTIL.debug_params(BRDG.get_world(), s);
  },

  // Handle "SAVE" button in Save modal.
  button_modal_save: function ()
  {
    let msg = VIEW.message_clear("save");
    let text = document.querySelector(".textarea.save").value;
    if (!text)
    {
      msg.classList.add("bad");
      msg.textContent = "Nothing to save";
      if (DEBUG) console.log("(button_modal_save) empty");
    }
    else
    {
      let ref = "data:text/plain;charset=utf-8," + text;
      let date = new Date();
      let d = date.getFullYear().toString() +
        ("0" + (date.getMonth() + 1)).slice(-2) +
        ("0" + date.getDate()).slice(-2) + date.getHours().toString() +
        date.getMinutes().toString() + date.getSeconds().toString();
      let a = document.createElement("a");
      a.href = ref;
      a.download = "emergence_save_" + d + ".txt";
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      msg.classList.add("good");
      msg.textContent = "Saved: " + a.download;
      if (DEBUG) console.log("(button_modal_save) saved");
    }
  },

  // Handle "COPY" button in Save modal.
  button_modal_copy: function ()
  {
    let area = document.querySelector(".textarea.save");
    area.focus();
    area.select();
    let msg = VIEW.message_clear("save");
    let copied = document.execCommand("copy");
    if (!area.value)
    {
      msg.classList.add("bad");
      msg.textContent = "Nothing to copy";
      if (DEBUG) console.log("(button_modal_copy) empty");
    }
    else if (copied)
    {
      msg.classList.add("good");
      msg.textContent = "Copied to clipboard";
      if (DEBUG) console.log("(button_modal_copy) copied");
    }
    else
    {
      msg.classList.add("bad");
      msg.textContent = "Copy failed";
      if (DEBUG) console.log("(button_modal_copy) copy failed");
    }
  },

  // Clear the message portion in Load/Save modals.
  message_clear: function (which)
  {
    let msg = document.querySelector(".message." + which);
    msg.classList.remove("good", "bad");
    msg.textContent = "";
    return msg;
  },

  // Reset modal states (on open/close).
  modal_reset: function (which, action)
  {
    let modal = document.querySelector(".modal." + which);
    if (modal.classList.contains("modal-show"))
    {
      document.addEventListener("keyup", VIEW.handle_key);
    }
    else
    {
      document.removeEventListener("keyup", VIEW.handle_key);
      setTimeout(action, 0);
    }
    modal.classList.toggle("modal-show");
  },

  // Handle selection of alpha/beta configuration.
  select_config: function ()
  {
    if (DEBUG) console.log("(select_config)");
    let config = BRDG.get_config(parseInt(
      document.getElementById("param_c").value));
    document.getElementById("param_a").value = config.a;
    document.getElementById("param_b").value = config.b;
    BRDG.load(config);
  },

  // Handle selection of color theme.
  select_theme: function ()
  {
    // TODO: something is awry here. when neighborhood surpasses 10,
    //       theme conversions become wacky.
    if (DEBUG) console.log("(select_theme)");
    let theme = BRDG.set_theme(parseInt(
      document.getElementById("param_t").value));
    BRDG.clear_theme_cache();
    document.getElementById("bar").style.backgroundColor = theme[0];
    document.querySelector("canvas").style.backgroundColor = theme[1];
    let spans = document.querySelectorAll("span.param");
    for (let i = 0; i < spans.length; i++)
    {
      spans[i].style.color = theme[2];
    }
  },

  // Handle selection of initial particle distribution scheme.
  select_distr: function ()
  {
    if (DEBUG) console.log("(select_distr)");
    let w = BRDG.get_world();
    VIEW.erase(w.c, w.w, w.h);
    BRDG.pause();
    BRDG.set_distr(parseInt(document.getElementById("param_o").value));
    BRDG.set_tick(0);
    VIEW.slider_set(0);
    VIEW.put_stop_calm();
    VIEW.put_params(CORE.get_params());
    VIEW.put_start();
    CORE.init();
  },

  // Handle user sliding of history slider.
  slider_load: function ()
  {
    BRDG.pause();
    BRDG.load(BRDG.get_history().snaps[this.value - 1]);
    BRDG.set_tick(this.value);
    CORE.init(true, true); // keep particles and radius
    BRDG.paint();
  },

  // Disable user-controllability of history slider.
  slider_disable: function ()
  {
    if (DEBUG) console.log("(slider_disable)");
    document.getElementById("slider").disabled = true;
  },

  // Enable user-controllability of history slider.
  slider_enable: function ()
  {
    if (DEBUG) console.log("(slider_enable)");
    document.getElementById("slider").disabled = false;
  },

  // Set history slider value (and knob).
  slider_set: function (tick)
  {
    document.getElementById("slider").value = tick;
  },

  // Make toggle button say "START".
  put_start: function ()
  {
    document.getElementById("button-bar-toggle").value = "START";
  },

  // Make stop parameter noticeable.
  put_stop_alert: function ()
  {
    document.getElementById("stop").classList.add("alert");
  },

  // Disable stop parameter alerting.
  put_stop_calm: function ()
  {
    document.getElementById("stop").classList.remove("alert");
  },

  // Set parameter fields to values of given abbreviated state object.
  put_params: function (o)
  {
    UTIL.for_abbrev(BRDG.get_abbrev(), function (key) {
      if (o[key] !== "")
      {
        document.getElementById("param_" + key).value =
          o[key].toString();
      }
    });
  },

  // Retrieve values of parameter fields.
  get_params: function ()
  {
    let o = {};
    let f;

    // TODO: improve int or float detection. don't hardcode
    UTIL.for_abbrev(BRDG.get_abbrev(), function (key) {
      if (["s", "o", "f", "n", "z"].includes(key))
      {
        f = parseInt;
      }
      else if (["r", "d", "a", "b", "g"].includes(key))
      {
        f = parseFloat;
      }
      o[key] = f(document.getElementById("param_" + key).value);
    })

    VIEW.fallback(o);

    return o;
  },

  // Helper for `button_modal_apply`.
  // - Called by the Load modal's "APPLY" button
  // - Calls `BRDG.load`.
  // - TODO: needs refactoring wrt. other ways of loading, ie. how
  //         bar's "APPLY" and "RESTART" buttons do it.
  load: function (o, keep)
  {
    let has = false;
    UTIL.for_abbrev(BRDG.get_abbrev(), function (key)
    {
      if (o[key])
      {
        has = true;
      }
    })
    if (!has) throw "unmembered state";
    VIEW.fallback(o, keep);
    VIEW.put_params(o);
    BRDG.load(o);
  },

  // Include default value if user input does not supply a parameter.
  fallback: function (o, keep)
  {
    if (!keep)
    {
      let a = BRDG.get_abbrev()
      let w = BRDG.get_world();
      let s = BRDG.get_state();
      let ds = BRDG.get_state_default();

      UTIL.for_abbrev(a, function (key)
      {
        if (isNaN(o[key]))
        {
          if (key === "r")
          {
            o[key] = UTIL.auto_radius(w, s);
          }
          else
          {
            o[key] = ds[a[key]];
          }
        }
      });
    }
  },
};

