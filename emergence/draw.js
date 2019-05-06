// file: draw.js
// by  : jooh@cuni.cz
// for : nprg045

////////////////////////////////////////////////////////////////////////
// draw, using html + css + canvas + javascript

let D =
{

  init: function ()
  {
    document.addEventListener("keyup", D.handle_key);
    if (window.File && window.FileReader && window.FileList && window.Blob)
    {
      document.getElementById("button-open-real")
        .addEventListener("change", D.handle_file);
      document.getElementById("button-open")
        .classList.add("button-show");
    }
  },

  paint: function (c, pts, n, w, h, rad, hue, avg, theme)
  {
    let p;
    c.clearRect(0, 0, w, h);
    for (let i = 0; i < n; i++)
    {
      p = pts[i];
      c.fillStyle = "hsl(" + hue(p.n, avg, theme) + ",100%,50%)";
      c.fillRect(p.x, p.y, rad, rad);
    }
  },

  toggle: function (paused, tick, fps)
  {
    let button = document.getElementById("button-pause");
    if (paused)
    {
      button.value = "PAUSE";
      animate = setInterval(tick, (1000 / fps));
      if (DEBUG) console.log("(toggle) play");
    }
    else
    {
      button.value = "RESUME";
      window.clearTimeout(animate);
      if (DEBUG) console.log("(toggle) pause");
    }
  },

  get_canvas: function ()
  {
    let c = document.querySelector("canvas");
    return {
      canvas: c,
      context: c.getContext("2d"),
    };
  },

  get_dimensions: function (gap)
  {
    return {
      w: window.innerWidth,
      h: window.innerHeight - gap,
    };
  },

  handle_key: function (event)
  {
    switch (event.key)
    {
      case " ":
        B.toggle();
        break;

      default:
        //if (DEBUG) console.log("(handle_key) unrecognised key");
    }
  },

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

  modal_help: function ()
  {
    document.querySelector(".modal.help")
      .classList.toggle("modal-show");
    B.pause();
  },

  modal_load: function ()
  {
    let msg = D.message_clear("load");
    let area = document.querySelector(".textarea.load");
    area.value = "";
    D.modal_prep("load", function()
    {
      area.focus();
    });
    B.pause();
  },

  modal_save: function ()
  {
    let msg = D.message_clear("save");
    let area = document.querySelector(".textarea.save");
    area.value = JSON.stringify(B.get_save(), null, 2);
    D.modal_prep("save", function()
    {
      area.focus();
      area.select();
      area.scrollTop = 0;
    });
    B.pause();
  },

  button_apply_bar: function()
  {
    let w = B.get_world();
    let s = B.get_state();
    w.fps = parseInt(document.getElementById("param_f").value);
    s.n = parseInt(document.getElementById("param_n").value);
    s.z = parseInt(document.getElementById("param_z").value);
    s.d = parseFloat(document.getElementById("param_d").value);
    s.a = parseFloat(document.getElementById("param_a").value);
    s.b = parseFloat(document.getElementById("param_b").value);
    s.g = parseFloat(document.getElementById("param_g").value);
    B.toggle(); // to enforce fps
    E.init();
    U.prep(s);
    B.resume();
    if (DEBUG) U.debug_params(w, s);
  },

  button_apply_load: function ()
  {
    let msg = D.message_clear("load");
    let text = document.querySelector(".textarea.load").value;
    let s = B.get_state();
    let parsed;
    try
    {
      parsed = JSON.parse(text);
      if (U.is_empty(parsed))
      {
        throw "empty state";
      }
      D.load(parsed);
    }
    catch(err)
    {
      msg.classList.add("bad");
      msg.textContent = 'Unparsable (see "HELP" on state)';
      if (DEBUG) console.log(err);
      if (DEBUG) console.log("(button_apply_load) parse & load failed");
      return;
    }
    D.modal_load();
    E.init(true); // do not create new particles
    U.prep(s);
    B.resume();
    if (DEBUG) console.log("(button_apply_load) applied");
    if (DEBUG) U.debug_params(B.get_world(), s);
  },

  button_copy: function ()
  {
    let area = document.querySelector(".textarea.save");
    area.focus();
    area.select();
    let msg = D.message_clear("save");
    let copied = document.execCommand("copy");
    if (!area.value)
    {
      msg.classList.add("bad");
      msg.textContent = "Nothing to copy";
      if (DEBUG) console.log("(button_copy) empty");
    }
    else if (copied)
    {
      msg.classList.add("good");
      msg.textContent = "Copied to clipboard";
      if (DEBUG) console.log("(button_copy) copied");
    }
    else
    {
      msg.classList.add("bad");
      msg.textContent = "Copy failed";
      if (DEBUG) console.log("(button_copy) copy failed");
    }
  },

  message_clear: function (which)
  {
    let msg = document.querySelector(".message." + which);
    msg.classList.remove("good", "bad");
    msg.textContent = "";
    return msg;
  },

  modal_prep: function (which, action)
  {
    let modal = document.querySelector(".modal." + which);
    if (modal.classList.contains("modal-show"))
    {
      document.addEventListener("keyup", D.handle_key);
    }
    else
    {
      document.removeEventListener("keyup", D.handle_key);
      setTimeout(action, 0);
    }
    modal.classList.toggle("modal-show");
  },

  say_restart: function ()
  {
    document.getElementById("button-pause").value = "RESTART";
  },

  load: function (o)
  {
    if (!(o.f || o.n || o.z || o.d || o.g || o.a || o.b || o.p))
    {
      throw "unmembered state";
    }
    if (o.f) document.getElementById("param_f").value = o.f.toString();
    if (o.n) document.getElementById("param_n").value = o.n.toString();
    if (o.z) document.getElementById("param_z").value = o.z.toString();
    if (o.d) document.getElementById("param_d").value = o.d.toString();
    if (o.a) document.getElementById("param_a").value = o.a.toString();
    if (o.b) document.getElementById("param_b").value = o.b.toString();
    if (o.g) document.getElementById("param_g").value = o.g.toString();
    B.load(o);
  },
};
