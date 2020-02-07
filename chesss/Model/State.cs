using System;
using System.Collections.Generic;
using System.Text.Json;

namespace Chesss.Model
{
  public class State //: IObservable
  {
    // public for json-serialisation
    public List<Piece> Lives { get; set; }
    public List<Piece> Deads { get; set; }
    public List<(string,long)> History { get; set; }

    public State()
    {
      var lives = new List<Piece>();
      this.Lives = lives;
      lives.Add(new Rook(this, Color.White, 1, 1));
      lives.Add(new Rook(this, Color.White, 8, 1));
      lives.Add(new Knight(this, Color.White, 2, 1));
      lives.Add(new Knight(this, Color.White, 7, 1));
      lives.Add(new Bishop(this, Color.White, 3, 1));
      lives.Add(new Bishop(this, Color.White, 6, 1));
      lives.Add(new Queen(this, Color.White, 4, 1));
      lives.Add(new King(this, Color.White, 5, 1));
      for (int file = 1; file <= 8; file++)
      {
        lives.Add(new Pawn(this, Color.White, file, 2));
      }
      for (int file = 1; file <= 8; file++)
      {
        lives.Add(new Pawn(this, Color.Black, file, 7));
      }
      lives.Add(new Rook(this, Color.Black, 1, 8));
      lives.Add(new Rook(this, Color.Black, 8, 8));
      lives.Add(new Knight(this, Color.Black, 2, 8));
      lives.Add(new Knight(this, Color.Black, 7, 8));
      lives.Add(new Bishop(this, Color.Black, 3, 8));
      lives.Add(new Bishop(this, Color.Black, 6, 8));
      lives.Add(new Queen(this, Color.Black, 4, 8));
      lives.Add(new King(this, Color.Black, 5, 8));
      this.Deads = new List<Piece>();
      this.History = new List<(string,long)>();
    }

    public string
    ToJson()
    {
      return JsonSerializer.Serialize<State>(this);
    }

    public Color
    Turn()
    {
      if (this.History.Count % 2 == 0)
      {
        return Color.Black;
      }
      return Color.White;
    }

    //public void Move()
  }

  public class Time
  {
    public long
    Now()
    {
      return DateTimeOffset.UtcNow.ToUnixTimeSeconds();
    }
  }
}
