using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading;
using Chesh.Controller;
using Chesh.Model;
using Chesh.Util;

namespace Chesh.View
{
  public class Ui
  {
    public static int HostWidth;
    public static int HostHeight;
    public Dictionary<string,Element> Es { get; set; }
    private Game Game;
    private Control Control;
    private string State;
    private string Cfg;
    private int Width;
    private int Height;
    private bool Running;

    public Ui(Game game)
    {
      this.Game = game;
      this.State = Helper.ToJson(game.State);
      this.Cfg = Helper.ToJson(game.Cfg);
      string style = Helper.FromJsonToCfgValue(this.Cfg, "style");
      this.Es = new Dictionary<string,Element>();
      this.Es["BlackResponse"] = new BlackResponseElement(style);
      this.Es["WhiteDead"] = new WhiteDeadElement(style);
      this.Es["Frame"] = new BoardFrameElement(style);
      this.Es["BlackDead"] = new BlackDeadElement(style);
      this.Es["WhiteResponse"] = new WhiteResponseElement(style);
      this.Es["History"] = new HistoryElement(style);
      this.Es["Pieces"] = new PiecesElement(style);
      this.Es["BlackPrompt"] = new BlackPromptElement(style);
      this.Es["WhitePrompt"] = new WhitePromptElement(style);
      this.Width = 26;
      this.Height = 19;
      HostWidth = Console.WindowWidth;
      HostHeight = Console.WindowHeight;
      if (style == "wide")
      {
      }
      this.Running = true;
    }

    public void
    SetControl(Control control)
    {
      this.Control = control;
    }

    public void
    Restate(string state)
    {
      this.State = state;
    }

    public static void
    Write(string s)
    {
      Console.Write(s);
    }

    public static void
    Pin(int x, int y)
    {
      Console.SetCursorPosition(x, y);
    }

    public void
    Run(List<string> load, float speed)
    {
      if (! this.GoodDimensions())
      {
        Console.WriteLine("Host must have at least width {0} and height {1}",
                          this.Width, this.Height);
        return;
      }
      Console.Clear();
      this.Draw();

      int count = 0;
      foreach (var note in load)
      {
        count++;
        string message = $"{note}: Move {count} is invalid!";
        if (note != "bye" && (note.Length < 5 || note.Length > 7))
        {
          this.Respond(message, true, true);
          break;
        }
        string move = Helper.Denotate(note);
        if (move == "bye")
        {
          this.Respond($"{this.Turn(true)} resigned. {this.Turn(false)} wins!",
                       true, true);
          this.Stop("Save before quitting?", true, false, false);
          Pin(0, this.Height + 1);
          return;
        }
        (int,int,int,int) srcdst = Helper.FromMoveToInts(move);
        var rets = this.Move(srcdst.Item1, srcdst.Item2,
                             srcdst.Item3, srcdst.Item4,
                             $"{move[0]}{move[1]}", $"{move[2]}{move[3]}",
                             true);
        foreach (var ret in rets)
        {
          if (! (new List<Ret>() {
                Ret.Capture,
                Ret.Castle,
                Ret.Promote,
                Ret.Check,
                Ret.Checkmate
             }).Contains(ret))
          {
            this.Respond(message, true, true);
          }
        }
        if (rets.Contains(Ret.Promote))
        {
          string prom = Regex.Replace(note.Substring(5), @"[*&#+:p%]",
                                      string.Empty);
          this.Control.Promote(rets, prom,
                               srcdst.Item1, srcdst.Item2,
                               srcdst.Item3, srcdst.Item4);
        }
        if (rets.Contains(Ret.Checkmate))
        {
          return;
        }
        if (speed > 0)
        {
          Thread.Sleep((int) (speed * 1000));
        }
      }

      string input;
      while (Running)
      {
        input = Read();
        switch (input)
        {
          case "":
            this.Reprompt(null, true);
            continue;
          case "exit":
          case "quit":
          case "stop":
            this.Stop("Save and quit?", true, true, true);
            Pin(0, this.Height + 1);
            break;
          case "forfeit":
          case "giveup":
          case "resign":
            this.Respond($"{this.Turn(true)} resigned. {this.Turn(false)} wins!",
                         false, false);
            break;
          default:
            this.Understand(input);
            break;
        }
      }
    }

    public void
    Stop(string message, bool current, bool responder, bool reprompt)
    {
      if (Confirm(message + " (y/n)", false, current, responder))
      {
        if (File.Exists("chesh.log") &&
            Confirm("chesh.log already exists. Sure? (y/n)",
                    false, current, true))
        {
          this.Running = false;
          this.Control.Save();
          this.Reprompt("Game log saved: chesh.log", current);
          return;
        }
      }
      if (reprompt)
      {
        this.Respond(null, current, true);
      }
    }

    public string
    Read()
    {
      string input = string.Empty;
      while (true)
      {
        ConsoleKeyInfo cki = Console.ReadKey(true);
        ConsoleKey k = cki.Key;
        char c = cki.KeyChar;
        if (k == ConsoleKey.Escape)
        {
          break;
        }
        if (k == ConsoleKey.Enter)
        {
          break;
        }
        if (k == ConsoleKey.Backspace)
        {
          if (input != "")
          {
            input = input.Substring(0, input.Length - 1);
            Write("\b \b");
          }
          continue;
        }
        if ((cki.Modifiers & ConsoleModifiers.Control) != 0 &&
            (k == ConsoleKey.D))
        {
          this.Reprompt("quit", true);
          return "quit";
        }
        if ((cki.Modifiers & ConsoleModifiers.Control) != 0
            || (cki.Modifiers & ConsoleModifiers.Alt) != 0)
        {
          continue;
        }
        if (input.Length < this.Es[this.Turn(true) + "Prompt"].Width)
        {
          Write(c.ToString());
          input += c;
        }
      }
      return input.Trim().ToLower();
    }

    public void
    Understand(string input)
    {
      input = Regex.Replace(input, @"\s+", string.Empty).ToLower();
      if (input.Length != 4)
      {
        this.Respond(input + " ?", true, true);
        return;
      }

      string src = $"{input[0]}{input[1]}";
      string dst = $"{input[2]}{input[3]}";
      if (! Helper.ValidSquare(input[0], input[1]))
      {
        this.Respond("Unrecognised square: " + src, true, true);
        return;
      }
      if (! Helper.ValidSquare(input[2], input[3]))
      {
        this.Respond("Unrecognised square: " + dst, true, true);
        return;
      }
      string name = Helper.FromSymToName(this.At(input[0], input[1]));
      if (name != null)
      {
        name += " ";
      }
      string move = $"{name}{src}->{dst}";
      if (! Confirm($"{move}? (y/n)", true, true, true))
      {
        this.Respond(null, true, true);
        return;
      }
      (int,int,int,int) srcdst = Helper.FromMoveToInts(input);
      this.Move(srcdst.Item1, srcdst.Item2,
                srcdst.Item3, srcdst.Item4,
                src, dst, false);
    }

    public void
    Respond(string message, bool current, bool reprompt)
    {
      this.Es[this.Turn(current) + "Response"].Erase();
      Write(message);
      if (reprompt)
      {
        this.Reprompt(null, current);
      }
    }

    public void
    Reprompt(string message, bool current)
    {
      var prompt = this.Es[this.Turn(current) + "Prompt"];
      prompt.Erase();
      prompt.Draw(message);
    }

    public bool
    Confirm(string message, bool enterIsYes, bool current, bool responder)
    {
      if (responder)
      {
        this.Respond(message + " ", current, false);
      }
      else
      {
        this.Reprompt(message + " ", current);
      }
      ConsoleKey k;
      while (true)
      {
        k = Console.ReadKey(true).Key;
        if (k == ConsoleKey.Y)
        {
          return true;
        }
        if (enterIsYes && k == ConsoleKey.Enter)
        {
          return true;
        }
        if (k == ConsoleKey.N)
        {
          return false;
        }
      }
    }

    public string
    ConfirmPromote(string prefix)
    {
      this.Respond($"{prefix} Promote to? (RNBQ) ", true, false);
      ConsoleKey k;
      while (true)
      {
        k = Console.ReadKey(true).Key;
        if (k == ConsoleKey.R)
        {
          return "R";
        }
        if (k == ConsoleKey.N)
        {
          return "N";
        }
        if (k == ConsoleKey.B)
        {
          return "B";
        }
        if (k == ConsoleKey.Q)
        {
          return "Q";
        }
      }
    }

    public string
    Turn(bool current)
    {
      int count = 0;
      foreach (var note in
               Helper.FromJsonToStateListValue(this.State, "History"))
      {
        count++;
      }
      if (count % 2 == 0)
      {
        if (current)
        {
          return "White";
        }
        return "Black";
      }
      if (current)
      {
        return "Black";
      }
      return "White";
    }

    public bool
    GoodDimensions()
    {
      //switch ()
      //{
      //  case CfgVal.StyleCompact:
      //    if (this.Width < 24 || this.Height < 20)
      //    {
      //      return CheckRet.BadDim;
      //    }
      //    break;
      //  case CfgVal.StyleWide:
      //    break;
      //  default:
      //    break;
      //}
      return true;
    }

    public void
    Draw()
    {
      foreach (KeyValuePair<string,Element> entry in this.Es)
      {
        if (! entry.Key.EndsWith("Response") && ! entry.Key.EndsWith("Prompt"))
        {
          entry.Value.Draw(this.State);
        }
      }
      this.Respond(null, false, true);
      this.Respond(null, true, true);
    }

    public string
    At(char file, char rank)
    {
      foreach (JsonElement piece in
               Helper.FromJsonToStateListValue(this.State, "Live"))
      {
        if (piece[2].GetInt32() == Helper.ToFileNum(file) &&
            piece[3].GetInt32() == Helper.ToRankNum(rank))
        {
          var sym = piece[0].GetString();
          return sym;
        }
      }
      return null;
    }

    public List<Ret>
    Move(int xSrc, int ySrc, int xDst, int yDst,
         string src, string dst, bool playback)
    {
      string prefix = src + "->" + dst + ":";
      List<Ret> rets = this.Control.Move(xSrc, ySrc, xDst, yDst);
      if (rets.Contains(Ret.BadSrc))
      {
        this.Respond($"{prefix} No piece on " + src, true, true);
        return rets;
      }
      if (rets.Contains(Ret.BadTurn))
      {
        this.Respond($"{prefix} Other player's turn", true, true);
        return rets;
      }
      if (rets.Contains(Ret.BadDst))
      {
        this.Respond($"{prefix} A friendly on " + dst, true, true);
        return rets;
      }
      if (rets.Contains(Ret.BadCastle))
      {
        this.Respond($"{prefix} Cannot castle", true, true);
        return rets;
      }
      if (rets.Contains(Ret.InvalidMove))
      {
        this.Respond($"{prefix} Illegal move", true, true);
        return rets;
      }
      if (rets.Contains(Ret.Checked))
      {
        this.Respond($"{prefix} King will be checked", true, true);
        return rets;
      }
      if (rets.Contains(Ret.Checkmate))
      {
        this.Respond($"Checkmate. {this.Turn(false)} wins!", false, true);
        this.Stop("Save before quitting?", false, false, false);
        Pin(0, this.Height + 1);
        return rets;
      }
      if (rets.Contains(Ret.Promote) && ! playback)
      {
        this.Control.Promote(rets, ConfirmPromote(prefix),
                             xSrc, ySrc, xDst, yDst);
      }
      // check Ret.Regular
      //       Ret.Capture
      //       Ret.Castle
      //       Ret.Promote
      //       Ret.EnPassant
      //       Ret.Check
      this.Reprompt(null, false);
      this.Reprompt(null, true);
      return rets;
      //this.Respond($"{prefix} Server error :-(", true, true);
    }
  }
}
