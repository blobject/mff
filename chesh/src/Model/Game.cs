using System;
using System.Collections.Generic;
using System.Text.Json;

namespace Chesh.Model
{
  public enum Color { None, Black, White }

  public class Game
  {
    public State State { get; set; }
    public bool BlackDrew { get; set; }
    public bool WhiteDrew { get; set; }
    public Color? Forfeiture { get; set; }
    public bool Over { get; set; }

    public Game()
    {
      this.State = new State();
      this.BlackDrew = false;
      this.WhiteDrew = false;
      this.Forfeiture = null;
      this.Over = false;
    }

    // for testing
    public Game(string strands) // "pieces (of) string"
    {
    }

    public string
    ToJson()
    {
      return JsonSerializer.Serialize<Game>(this);
    }

    public void
    Select(int x, int y)
    {
      this.State.Selection = this.State.At(x, y);
    }

    public void
    Draw(Color color)
    {
      if (color == Color.Black)
      {
        this.BlackDrew = true;
      }
      else
      {
        this.WhiteDrew = true;
      }
    }

    public void
    Undraw(Color color)
    {
      if (color == Color.Black)
      {
        this.BlackDrew = false;
      }
      else
      {
        this.WhiteDrew = false;
      }
    }

    public void
    Forfeit(Color color)
    {
      this.Forfeiture = color;
      this.Over = true;
    }
  }
}
