using System;
using System.Collections.Generic;
using System.IO;
using Chesh.Util;

namespace Chesh.Model
{
  public enum Color { Black, White }

  public enum Ret
  {
    Selected,
    Castle,
    Capture,
    Promote,
    EnPassant,
    Check,
    Checkmate,
    Checked,
    BadSrc,
    BadDst,
    BadTurn,
    BadCastle,
    InvalidMove,
  }

  public class Game : ISubject
  {
    private List<IObserver> _observers;
    private State _state;
    public State State
    {
      get
      {
        return _state;
      }
      set
      {
        StateChanged(value);
        _state = value;
      }
    }
    public Dictionary<string,string> Cfg { get; set; }
    public bool BlackWantsTie { get; set; }
    public bool WhiteWantsTie { get; set; }
    public Color? Resigned { get; set; }
    public bool Over { get; set; }

    public Game()
    {
      _observers = new List<IObserver>();
      this.State = new State();
      this.BlackWantsTie = false;
      this.WhiteWantsTie = false;
      this.Resigned = null;
      this.Over = false;
      this.Cfg = new Dictionary<string,string>();
      this.Cfg["style"] = "compact";
      this.Cfg["quit"] = "no";
    }

    // accessors ///////////////////////////////////////////////////////////////

    public Piece
    At(int x, int y)
    {
      return this.State.At(x, y);
    }

    // mutators ////////////////////////////////////////////////////////////////

    public void
    Attach(IObserver observer)
    {
      _observers.Add(observer);
    }

    public void
    Tie(Color color)
    {
      if (color == Color.Black)
      {
        this.BlackWantsTie = true;
      }
      else
      {
        this.WhiteWantsTie = true;
      }
    }

    public void
    Untie(Color color)
    {
      if (color == Color.Black)
      {
        this.BlackWantsTie = false;
      }
      else
      {
        this.WhiteWantsTie = false;
      }
    }

    public void
    Resign(Color color)
    {
      this.Resigned = color;
      this.Over = true;
    }

    // M -> notify C ///////////////////////////////////////////////////////////

    public void
    StateChanged(State next)
    {
      foreach (var observer in _observers)
      {
        observer.ChangeState(next);
      }
    }

    // player actions //////////////////////////////////////////////////////////

    public void
    SetStyleCfg(string choice)
    {
      this.Cfg["style"] = choice;
    }

    public void
    SetQuitCfg(string choice)
    {
      this.Cfg["quit"] = choice;
    }

    // TODO: unused
    public Ret
    Select(int x, int y)
    {
      return this.State.Select(x, y);
    }

    public void
    Promote(List<Ret> rets, string prom, int xSrc, int ySrc, int xDst, int yDst)
    {
      this.State.Promote(rets, prom, xSrc, ySrc, xDst, yDst);

      // trigger the observer
      this.State = this.State;
    }

    public List<Ret>
    Move(int xSrc, int ySrc, int xDst, int yDst)
    {
      var rets = this.State.Move(xSrc, ySrc, xDst, yDst);

      // trigger the observer
      this.State = this.State;

      return rets;
    }

    public void
    Save()
    {
      int count = 0;
      string last = new string(' ', 8);
      using (var writer = new StreamWriter("chesh.log"))
      {
        writer.WriteLine("# " + (new DateTime(DateTime.Now.Ticks)).Ticks);
        foreach (var note in this.State.History)
        {
          if (count % 2 == 0)
          {
            writer.Write(note.Item1);
          }
          else
          {
            writer.Write(new string(' ', 8 - last.Length));
            writer.WriteLine(note.Item1);
          }
          count++;
          last = note.Item1;
        }
        if (this.State.History.Count % 2 == 1)
        {
          writer.WriteLine();
        }
        writer.WriteLine();
      }
    }
  }
}
