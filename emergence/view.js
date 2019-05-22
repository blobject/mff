// file: emergence/view.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// draw, using html + css + canvas + javascript

const VIEW =
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

  // Reinitialise View.
  reinit: function (clear)
  {
    const w = BRDG.get_world();
    const s = BRDG.get_state();
    if (clear)
    {
      BRDG.set_tick(0); // which also disables slider via VIEW.toggle()
      BRDG.clear_theme_cache();
      VIEW.erase(w.context, s.width, s.height);
    }
    if (w.tick < s.stop)
    {
      VIEW.put_stop_calm();
    }
  },

  // Load parameters, reset view, and continue (or start) drawing.
  start: function (raw, fresh, clear, pause)
  {
    VIEW.reinit(clear);
    const params = CORE.sanitise(raw, fresh);
    const s = BRDG.get_state();
    const sw = s.width / params.w;
    const sh = s.height / params.h;
    CORE.set(params);
    CORE.init(fresh);
    VIEW.scale(BRDG.get_world().context, s.width, s.height, sw, sh);
    VIEW.put_params(params);
    BRDG.pause(); // to register new fps
    if (!pause)
    {
      BRDG.resume();
    }
    if (DEBUG) { UTIL.debug_params(BRDG.get_world(), BRDG.get_state()); }
  },

  // Pause or resume animation.
  toggle: function (paused, tick, fps)
  {
    const button = document.getElementById("button-bar-toggle");
    if (paused)
    {
      button.value = "PAUSE";
      animate = setInterval(tick, (1000 / fps));
      if (WORLD.tick < WORLD.history_size)
      {
        VIEW.slider_disable();
      }
      if (DEBUG) { console.log("(toggle) play"); }
    }
    else
    {
      button.value = "RESUME";
      window.clearTimeout(animate);
      if (DEBUG) { console.log("(toggle) pause"); }
    }
  },

  // Clear canvas.
  erase: function (context, width, height)
  {
    context.clearRect(0, 0, width, height);
  },

  // Draw particles on canvas.
  paint: function (context, pts, n, width, height, size, hue, density,
                   theme)
  {
    let p;
    VIEW.erase(context, width, height);
    for (let i = 0; i < n; i++)
    {
      p = pts[i];
      context.fillStyle = "hsl(" + hue(p.n, density, theme[0]) +
                          ",100%,50%)";
      if (p.n === 0)
      {
        context.fillStyle = theme[3] + "40";
      }
      context.fillRect(p.x, p.y, size, size);
    }
  },

  // Scale the canvas.
  scale: function (context, width, height, scaled_width, scaled_height)
  {
    VIEW.erase(context, width, height);
    context.scale(scaled_width, scaled_height);
  },

  // Hand over HTML canvas object and its 2D context.
  get_canvas: function ()
  {
    const c = document.querySelector("canvas");
    return {
      canvas: c,
      context: c.getContext("2d"),
    };
  },

  // Hand over page dimensions.
  get_dimensions: function ()
  {
    return {
      width: window.innerWidth,
      height: window.innerHeight - BRDG.get_world().gap,
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
        //if (DEBUG) { console.log("(handle_key) unrecognised key"); }
    }
  },

  // Handle file open event.
  handle_file: function (event)
  {
    const r = new FileReader();
    r.onload = function()
    {
      const area = document.querySelector(".textarea.load");
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
    const msg = VIEW.message_clear("load");
    const area = document.querySelector(".textarea.load");
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
    const msg = VIEW.message_clear("save");
    const area = document.querySelector(".textarea.save");
    const dim = VIEW.get_dimensions();
    area.value = CORE.encode(dim.width, dim.height,
                             CORE.get_snapshot());
    VIEW.modal_reset("save", function()
    {
      area.focus();
      area.select();
      area.scrollTop = 0;
    });
    BRDG.pause();
  },

  // Handle "RANDOM" button on bar.
  button_bar_random: function ()
  {
    VIEW.start(Object.assign(VIEW.get_params(), UTIL.randomise()),
               true, true);
  },

  // Handle "RESTART" button on bar.
  button_bar_restart: function ()
  {
    VIEW.start(VIEW.get_params(), true, true);
  },

  // Handle "APPLY" button on bar.
  button_bar_apply: function()
  {
    VIEW.start(VIEW.get_params());
  },

  // Handle "APPLY" button in Load modal.
  button_modal_apply: function ()
  {
    try
    {
      const parsed = CORE.decode(
        document.querySelector(".textarea.load").value);
      if (UTIL.is_empty(parsed))
      {
        throw "empty state";
      }
      VIEW.button_bar_load();
      VIEW.start(parsed, !!parsed.w || !!parsed.h, true);
      if (DEBUG) { console.log("(button_modal_apply) applied"); }
    }
    catch(err)
    {
      const msg = VIEW.message_clear("load");
      msg.classList.add("bad");
      msg.textContent = "Unparsable (see \"HELP\" on state)";
      console.log(err);
      if (DEBUG) { console.log("(button_modal_apply) parse & load failed"); }
    }
  },

  // Handle "SAVE" button in Save modal.
  button_modal_save: function ()
  {
    const msg = VIEW.message_clear("save");
    const text = document.querySelector(".textarea.save").value;
    if (!text)
    {
      msg.classList.add("bad");
      msg.textContent = "Nothing to save";
      if (DEBUG) { console.log("(button_modal_save) empty"); }
    }
    else
    {
      const ref = "data:text/plain;charset=utf-8," + text;
      const date = new Date();
      const d = date.getFullYear().toString() +
        ("0" + (date.getMonth() + 1)).slice(-2) +
        ("0" + date.getDate()).slice(-2) + date.getHours().toString() +
        date.getMinutes().toString() + date.getSeconds().toString();
      const a = document.createElement("a");
      a.href = ref;
      a.download = "emergence_save_" + d + ".txt";
      document.body.appendChild(a);
      a.click(); // a file save trick
      document.body.removeChild(a);
      msg.classList.add("good");
      msg.textContent = "Saved: " + a.download;
      if (DEBUG) { console.log("(button_modal_save) saved"); }
    }
  },

  // Handle "COPY" button in Save modal.
  button_modal_copy: function ()
  {
    const area = document.querySelector(".textarea.save");
    area.focus();
    area.select();
    const msg = VIEW.message_clear("save");
    const copied = document.execCommand("copy");
    if (!area.value)
    {
      msg.classList.add("bad");
      msg.textContent = "Nothing to copy";
      if (DEBUG) { console.log("(button_modal_copy) empty"); }
    }
    else if (copied)
    {
      msg.classList.add("good");
      msg.textContent = "Copied to clipboard";
      if (DEBUG) { console.log("(button_modal_copy) copied"); }
    }
    else
    {
      msg.classList.add("bad");
      msg.textContent = "Copy failed";
      if (DEBUG) { console.log("(button_modal_copy) copy failed"); }
    }
  },

  // Clear the message portion in Load/Save modals.
  message_clear: function (which)
  {
    const msg = document.querySelector(".message." + which);
    msg.classList.remove("good", "bad");
    msg.textContent = "";
    return msg;
  },

  // Reset modal states (on open/close).
  modal_reset: function (which, action)
  {
    const modal = document.querySelector(".modal." + which);
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
    if (DEBUG) { console.log("(select_config)"); }
    const config = BRDG.get_config(parseInt(
      document.getElementById("param_c").value));
    const params = CORE.sanitise(
      Object.assign(VIEW.get_params(), config));
    CORE.set(params);
    VIEW.put_params(params);
  },

  // Handle selection of color theme.
  select_theme: function ()
  {
    // TODO: when neighborhood surpasses 10, theme conversions become
    //       wacky, probably due to caching: BRDG.hue().
    if (DEBUG) { console.log("(select_theme)"); }
    const theme = BRDG.set_theme(parseInt(
      document.getElementById("param_x").value));
    BRDG.clear_theme_cache();
    document.getElementById("bar").style.backgroundColor = theme[1];
    document.querySelector("canvas").style.backgroundColor = theme[2];
    const spans = document.querySelectorAll("span.param");
    for (let i = 0; i < spans.length; i++)
    {
      spans[i].style.color = theme[3];
    }
    BRDG.paint();
  },

  // Handle selection of initial particle distribution scheme.
  select_distribution: function ()
  {
    if (DEBUG) { console.log("(select_distribution)"); }
    VIEW.start(VIEW.get_params(), true, true);
  },

  // Handle user sliding of history slider.
  slider_load: function ()
  {
    BRDG.pause();
    CORE.set(CORE.sanitise(BRDG.get_history().snaps[this.value - 1]));
    BRDG.set_tick(this.value);
    BRDG.paint();
  },

  // Disable user-controllability of history slider.
  slider_disable: function ()
  {
    if (DEBUG) { console.log("(slider_disable)"); }
    document.getElementById("slider").disabled = true;
  },

  // Enable user-controllability of history slider.
  slider_enable: function ()
  {
    if (DEBUG) { console.log("(slider_enable)"); }
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
      if (["w", "h"].includes(key))
      {
        return;
      }
      document.getElementById("param_" + key).value = o[key].toString();
    });
  },

  // Retrieve values of parameter fields.
  get_params: function ()
  {
    const o = {};
    let f;

    UTIL.for_abbrev(BRDG.get_abbrev(), function (key) {
      if (["w", "h"].includes(key))
      {
        return;
      }
      if (["t", "o", "f", "n", "r"].includes(key))
      {
        f = parseInt;
      }
      else if (["z", "d", "a", "b", "g"].includes(key))
      {
        f = parseFloat;
      }
      o[key] = f(document.getElementById("param_" + key).value);
    });

    return o;
  },
};

