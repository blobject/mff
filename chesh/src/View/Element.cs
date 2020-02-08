using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json;
using Chesh.Model;
using Chesh.Util;

namespace Chesh.View
{
  public abstract class Element
  {
    public int X;
    public int Y;
    public int Width;
    public int Height;

    public void
    Erase()
    {
      int width = this.Width;
      if ((new List<string>() {
            "BlackResponseElement",
            "WhiteResponseElement",
            "BlackPromptElement",
            "WhitePromptElement"
          }).Contains(this.GetType().Name))
      {
        width = Ui.HostWidth;
      }
      for (int i = 0; i < this.Height; i++)
      {
        Ui.Pin(this.X, this.Y + i);
        Ui.Write(new string(' ', width));
      }
      Ui.Pin(this.X, this.Y);
    }

    public abstract void Draw(string state);
  }

  public class ResponseElement : Element
  {
    public ResponseElement(string cfg)
    {
      this.X = 0;
      this.Width = 24;
      this.Height = 1;
    }

    public override void
    Draw(string message)
    {
      if (message == null)
      {
        return;
      }
      Ui.Pin(this.X, this.Y);
      Ui.Write(message);
    }
  }

  public class BlackResponseElement : ResponseElement
  {
    public BlackResponseElement(string cfg) : base(cfg)
    {
      this.Y = 0;
    }
  }

  public class WhiteResponseElement : ResponseElement
  {
    public WhiteResponseElement(string cfg) : base(cfg)
    {
      this.Y = 19;
    }
  }

  public abstract class PromptElement : Element
  {
    public PromptElement(string cfg)
    {
      this.X = 0;
      this.Width = 22;
      this.Height = 1;
    }

    public override abstract void Draw(string s);
  }

  public class BlackPromptElement : PromptElement
  {
    public BlackPromptElement(string cfg) : base(cfg)
    {
      this.Y = 1;
      //if (cfg == CfgVal.StyleWide)
      //{
      //  this.Y = 2;
      //}
    }

    public override void
    Draw(string s)
    {
      Ui.Pin(this.X, this.Y);
      Ui.Write("b> ");
      if (s != null)
      {
        Ui.Write(s);
      }
    }
  }

  public class WhitePromptElement : PromptElement
  {
    public WhitePromptElement(string cfg) : base(cfg)
    {
      this.Y = 18;
    }

    public override void
    Draw(string s)
    {
      Ui.Pin(this.X, this.Y);
      Ui.Write("w> ");
      if (s != null)
      {
        Ui.Write(s);
      }
    }
  }

  public abstract class DeadElement : Element
  {
    public DeadElement(string cfg)
    {
      this.X = 14;
      this.Width = 10;
      this.Height = 2;
    }

    public override abstract void Draw(string state);
  }

  public class WhiteDeadElement : DeadElement
  {
    public WhiteDeadElement(string cfg) : base(cfg)
    {
      this.Y = 3;
    }

    public override void
    Draw(string state)
    {
      Ui.Pin(this.X, this.Y);
      Ui.Write("x");
      Ui.Pin(this.X + 11, this.Y);
      Ui.Write("x");
      Ui.Pin(this.X + 2, this.Y);
      int count = 0;
      foreach (JsonElement piece in
               Helper.FromJsonToStateListValue(state, "Dead"))
      {
        if (! piece[1].GetBoolean()) // white
        {
          if (count >= 8)
          {
            Ui.Pin(this.X + 17 - count, this.Y - 1);
          }
          Ui.Write(piece[0].GetString()); // sym
          count++;
        }
      }
    }
  }

  public class BlackDeadElement : DeadElement
  {
    public BlackDeadElement(string cfg) : base(cfg)
    {
      this.Y = 16;
    }

    public override void
    Draw(string state)
    {
      Ui.Pin(this.X, this.Y);
      Ui.Write("x");
      Ui.Pin(this.X + 11, this.Y);
      Ui.Write("x");
      Ui.Pin(this.X + 2, this.Y);
      int count = 0;
      foreach (JsonElement piece in
               Helper.FromJsonToStateListValue(state, "Dead"))
      {
        if (piece[1].GetBoolean()) // black
        {
          if (count >= 8)
          {
            Ui.Pin(this.X + 17 - count, this.Y + 1);
          }
          Ui.Write(piece[0].GetString()); // sym
          count++;
        }
      }
    }
  }

  public class BoardFrameElement : Element
  {
    public BoardFrameElement(string cfg)
    {
      this.X = 14;
      this.Y = 4;
      this.Width = 12;
      this.Height = 12;
    }

    public override void
    Draw(string state)
    {
      int boardWidth = this.Width - 4;

      Ui.Pin(this.X, this.Y);
      Ui.Write("  ");
      for (int file = 1; file <= 8; file++)
      {
        Ui.Write(this.ToFileAlpha(file));
      }
      Ui.Pin(this.X, this.Y + 1);
      Ui.Write(" .");
      for (int i = 0; i < boardWidth; i++)
      {
        Ui.Write("-");
      }
      Ui.Write(".");
      Ui.Pin(this.X, this.Y + 2);
      for (int rank = boardWidth; rank >= 1; rank--)
      {
        Ui.Write(rank + "|");
        Ui.Write(new string(' ', boardWidth));
        Ui.Write("|" + rank);
        Ui.Pin(this.X, this.Y + this.Height - 1 - rank);
      }
      Ui.Pin(this.X, this.Y + this.Width - 2);
      Ui.Write(" '");
      for (int i = 0; i < boardWidth; i++)
      {
        Ui.Write("-");
      }
      Ui.Write("'");
      Ui.Pin(this.X, this.Y + this.Width - 1);
      Ui.Write("  ");
      for (int file = 1; file <= 8; file++)
      {
        Ui.Write(this.ToFileAlpha(file));
      }
    }

    public string
    ToFileAlpha(int file)
    {
      return ((char) (file + 'a' - 1)).ToString();
    }
  }

  public class HistoryElement : Element
  {
    public HistoryElement(string cfg)
    {
      this.X = 0;
      this.Y = 2;
      this.Width = 13;
      this.Height = 16;
    }

    public override void
    Draw(string state)
    {
      var history = Helper.FromJsonToStateListValue(state, "History");
      if (history == null)
      {
        return;
      }
      var rev = new Stack<string>();
      var stack = new Stack<string>();
      int num = this.Height * 2 - history.Count() % 2;
      int count = 0;
      foreach (JsonElement note in history)
      {
        // TODO: use time data
        rev.Push(note[0].GetString());
      }
      this.Erase();
      while (rev.Count != 0 && count < num)
      {
        count++;
        stack.Push(rev.Pop());
      }
      count = 0;
      foreach (var note in stack)
      {
        if (count % 2 == 0)
        {
          Ui.Pin(this.X, this.Y + (int) count / 2);
        }
        else
        {
          Ui.Pin(this.X + 7, this.Y + (int) count / 2);
        }
        Ui.Write(note);
        count++;
      }
    }
  }

  public class PiecesElement : Element
  {
    public PiecesElement(string cfg)
    {
      this.X = 16;
      this.Y = 6;
      this.Width = 8;
      this.Height = 8;
    }

    public override void
    Draw(string state)
    {
      Ui.Pin(this.X, this.Y);
      foreach (JsonElement piece in
               Helper.FromJsonToStateListValue(state, "Live"))
      {
        var sym = piece[0].GetString();
        var x = piece[2].GetInt32();
        var y = piece[3].GetInt32();
        Ui.Pin(this.X + x - 1, this.Y + 8 - y);
        Ui.Write(sym);
      }
    }
  }
}
