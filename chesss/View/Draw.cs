using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Chesss.Controller;
using Chesss.Model;
using Chesss.Util;

namespace Chesss.View
{
  public enum CheckRet { Good, BadDim }

  public enum Response
  {
    Move,
    ConfirmQuit,
    Quit,
    Empty,
    InvalidPiece,
    InvalidSrc,
    InvalidDst,
    BadSrc,
    BadDst,
    BadTurn,
    BadMove,
    Unknown
  }

  public class Point
  {
    public int X { get; set; }
    public int Y { get; set; }

    public Point(int x, int y)
    {
      this.X = x;
      this.Y = y;
    }

    public Point
    N()
    {
      int y = this.Y - 1;
      if (y < 0)
      {
        return null;
      }
      return new Point(this.X, y);
    }

    public Point
    E()
    {
      int x = this.X + 1;
      return new Point(x, this.Y);
    }

    public Point
    S()
    {
      int y = this.Y + 1;
      return new Point(this.X, y);
    }

    public Point
    W()
    {
      int x = this.X - 1;
      if (x < 0)
      {
        return null;
      }
      return new Point(x, this.Y);
    }
  }

  public class Ui
  {
    public Game Game { get; set; }
    public Move Move { get; set; }
    public bool Running { get; set; }
    public Point Point { get; set; }
    public int Width { get; set; }
    public int Height { get; set; }

    public Ui(Move move)
    {
      this.Game = move.Game;
      this.Move = move;
      this.Running = false;
      this.Width = Console.WindowWidth;
      this.Height = Console.WindowHeight;
    }

    public static void
    Pin(Point point)
    {
      Console.SetCursorPosition(point.X, point.Y);
    }

    public static void
    Draw(string s)
    {
      Console.Write(s);
    }

    public static void
    Draw(char c)
    {
      Console.Write(c);
    }

    public void
    Run()
    {
      CheckRet ret = Check();
      if (ret == CheckRet.BadDim)
      {
        Console.WriteLine("Terminal must be at least 22 by 22");
        return;
      }

      // TODO: check terminal resize

      this.Running = true;
      Console.Clear();
      DrawCompact();

      while (Running)
      {
        string input = GetInput();
        switch (input)
        {
          case "":
            Erase(new Point(0, 20), -1);
            Pin(new Point(2, 19));
            continue;
          case "quit":
          case "exit":
          case "bye":
            Stop();
            break;
          default:
            EvalInput(input);
            break;
        }
      }
    }

    public CheckRet
    Check()
    {
      if (this.Width < 22 || this.Height < 22)
      {
        return CheckRet.BadDim;
      }
      return CheckRet.Good;
    }

    public string
    GetInput()
    {
      string input = string.Empty;

      while (true)
      {
        ConsoleKeyInfo cki = Console.ReadKey(true);
        ConsoleKey k = cki.Key;
        char c = cki.KeyChar;

        if (k == ConsoleKey.Enter)
        {
          break;
        }

        if (k == ConsoleKey.Backspace)
        {
          if (input != "")
          {
            input = input.Substring(0, input.Length - 1);
            Draw("\b \b");
          }
          continue;
        }

        if ((cki.Modifiers & ConsoleModifiers.Control) != 0
            && (k == ConsoleKey.D))
        {
          Draw("bye");
          return "bye";
        }

        if ((cki.Modifiers & ConsoleModifiers.Control) != 0
            || (cki.Modifiers & ConsoleModifiers.Alt) != 0)
        {
          continue;
        }

        if (input.Length < 20)
        {
          Draw(c);
          input += c;
        }
      }

      return input.Trim().ToLower();
    }

    public void
    EvalInput(string input)
    {
      // TODO: help

      input = Regex.Replace(input, @"\s+", "").ToLower();

      if (input == string.Empty)
      {
        DrawCompactResponse(Response.Empty, input);
        return;
      }

      else if (input.Length != 5)
      {
        DrawCompactResponse(Response.Unknown, input);
        return;
      }

      string piece = input[0].ToString();
      string src = input.Substring(1, 2);
      string dst = input.Substring(3, 2);
      Move move = this.Move;
      MoveRet ret;
      Place srcPlace;
      Place dstPlace;

      if (!Helper.IsPieceSym(piece))
      {
        DrawCompactResponse(Response.InvalidPiece, piece);
        return;
      }

      if (!Helper.IsPlace(src))
      {
        DrawCompactResponse(Response.InvalidSrc, src);
        return;
      }

      if (!Helper.IsPlace(dst))
      {
        DrawCompactResponse(Response.InvalidDst, dst);
        return;
      }

      srcPlace = Helper.ToPlace(src);
      dstPlace = Helper.ToPlace(dst);
      ret = move.Try(srcPlace, dstPlace);

      if (ret == MoveRet.BadSrc)
      {
        DrawCompactResponse(Response.BadSrc, input);
        return;
      }

      if (ret == MoveRet.BadDst)
      {
        DrawCompactResponse(Response.BadDst, input);
        return;
      }

      if (ret == MoveRet.BadTurn)
      {
        DrawCompactResponse(Response.BadTurn, input);
        return;
      }

      if (ret == MoveRet.BadMove)
      {
        DrawCompactResponse(Response.BadMove, input);
        return;
      }

      if (DrawCompactResponse(Response.Move, input))
      {
        move.It(srcPlace, dstPlace);
        Erase(new Point(0, 20), -1);
        Reprompt();
        DrawCompact();
      }
      else
      {
        Erase(new Point(0, 20), -1);
        Reprompt();
      }
    }

    public void
    Stop()
    {
      if (DrawCompactResponse(Response.ConfirmQuit, null))
      {
        this.Running = false;
        DrawCompactResponse(Response.Quit, null);
        Console.WriteLine();
        return;
      }
      Erase(new Point(0, 20), -1);
      Reprompt();
    }

    public void
    Reprompt()
    {
      Point prompt = new Point(0, 19);
      Erase(prompt, -1);
      DrawCompactPrompt(prompt);
    }

    public void
    DrawCompact()
    {
      DrawCompactInfo(new Point(0, 0));
      DrawCompactCaptives(new Point(0, 3), this.Game.Other());
      DrawCompactBoard(new Point(0, 4), this.Game.Turn);
      DrawCompactCaptives(new Point(0, 16), this.Game.Turn);
      DrawCompactLog(new Point(13, 0));
      DrawCompactPieces(new Point(2, 6));
      DrawCompactPrompt(new Point(0, 19));
    }

    public void
    DrawCompactCaptives(Point origin, C color)
    {
      Pin(origin);
      string sym;
      foreach (Piece captive in this.Game.Captives(color))
      {
        sym = captive.Sym;
        if (color == C.White)
        {
          sym = sym.ToUpper();
        }
        Draw(sym);
      }
    }

    public void
    DrawCompactBoard(Point origin, C turn)
    {
      Point point = origin;
      int width = Board.Width;
      int height = Board.Height;

      Pin(point);
      Draw("  ");
      var nums = Enumerable.Range(1, width).ToArray();
      var revnums = new int[width];
      Array.Copy(nums, revnums, width);
      Array.Reverse(revnums);
      if (turn == C.Black)
      {
        Array.Reverse(nums);
        Array.Reverse(revnums);
      }
      foreach (var file in nums)
      {
        Draw(Helper.ToFileChar(file));
      }

      point = point.S();
      Pin(point);
      Draw(" .");
      for (int i = 1; i <= width; i++)
      {
        Draw("-");
      }
      Draw(".");

      point = point.S();
      Pin(point);
      foreach (var rank in revnums)
      {
        Draw(rank + "|");
        Draw(new string(' ', width));
        Draw("|" + rank);
        Console.WriteLine();
        point = point.S();
      }

      Draw(" '");
      for (int i = 1; i <= width; i++)
      {
        Draw("-");
      }
      Draw("'");

      point = point.S();
      Pin(point);
      Draw("  ");
      foreach (var file in nums)
      {
        Draw(Helper.ToFileChar(file));
      }
    }

    public void
    DrawCompactInfo(Point origin)
    {
      Pin(origin);
      Draw("00:00");
      Draw(" ");
      Draw("P");
    }

    public void
    DrawCompactLog(Point origin)
    {
      Pin(origin);
      Draw("log");
    }

    public void
    DrawCompactPrompt(Point origin)
    {
      Pin(origin);
      Draw("> ");
      Pin(new Point(origin.X + 2, origin.Y));
    }

    public bool
    DrawCompactResponse(Response res, string input)
    {
      Erase(new Point(0, 20), -1);

      if (res == Response.Move)
      {
        Draw($"Move {input}?");
        ConsoleKey k;
        while (true)
        {
          k = Console.ReadKey(true).Key;
          if (k == ConsoleKey.Y) { return true; }
          if (k == ConsoleKey.N) { return false; }
        }
      }

      else if (res == Response.ConfirmQuit)
      {
        Draw("Really quit? (y/n) ");
        ConsoleKey k;
        while (true)
        {
          k = Console.ReadKey(true).Key;
          if (k == ConsoleKey.Y) { return true; }
          if (k == ConsoleKey.N) { return false; }
        }
      }

      else if (res == Response.Quit)
      {
        Draw("Game log saved: chesss.log");
        return true;
      }

      else if (res == Response.Empty)
      {
        Draw("Input is empty");
      }

      else if (res == Response.InvalidPiece)
      {
        Draw("Unrecognised piece: " + input);
      }

      else if (res == Response.InvalidSrc || res == Response.InvalidDst)
      {
        Draw("Unrecognised place: " + input);
      }

      else if (res == Response.BadSrc)
      {
        Draw("Bad source: " + input);
      }

      else if (res == Response.BadDst)
      {
        Draw("Bad destination: " + input);
      }

      else if (res == Response.BadTurn)
      {
        Draw($"It is not {this.Game.Other()}'s turn");
      }

      else if (res == Response.BadMove)
      {
        Draw("Bad move: " + input);
      }

      else if (res == Response.Unknown)
      {
        Draw(input + " ?");
      }

      Reprompt();
      return true;
    }

    public void
    Erase(Point origin, int spaces)
    {
      if (spaces < 0)
      {
        spaces = this.Width - origin.X;
      }
      Pin(origin);
      Draw(new string(' ', spaces));
      Pin(origin);
    }

    public void
    DrawPiece(bool enemy, Point point, Piece piece)
    {
      if (enemy)
      {
        if (piece.Color == C.Black)
        {
          Pin(new Point(point.X + piece.Place.File - 1,
                        point.Y + (8 - piece.Place.Rank)));
        }
        else
        {
          Pin(new Point(point.X + piece.Place.File - 1,
                        point.Y + piece.Place.Rank - 1));
        }
      }
      else
      {
        if (piece.Color == C.Black)
        {
          Pin(new Point(point.X + piece.Place.File - 1,
                        point.Y + 7 + (piece.Place.Rank - 8)));
        }
        else
        {
          Pin(new Point(point.X + piece.Place.File - 1,
                        point.Y + 8 + (piece.Place.Rank * -1)));
        }
      }
      string sym = piece.Sym;
      if (piece.Color == C.White)
      {
        sym = sym.ToUpper();
      }
      if (piece.Color == C.Black
          && typeof(Pawn).IsInstanceOfType(piece))
      {
        sym = ((char) (sym[0] - 1)).ToString();
      }
      Draw(sym);
    }

    public void
    DrawCompactPieces(Point origin)
    {
      Pin(origin);
      Game game = this.Game;
      List<Piece> enemy = game.Hand(game.Other());
      List<Piece> hand = game.Hand(game.Turn);
      foreach (var piece in enemy)
      {
        DrawPiece(true, origin, piece);
      }
      foreach (var piece in hand)
      {
        DrawPiece(false, origin, piece);
      }
    }
  }
}
