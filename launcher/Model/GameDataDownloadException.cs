using System;

namespace CNC.NCO.Launcher.Model;

#pragma warning disable CS8618 // YAML deserialization populates these members
public class GameDataDownloadException : Exception
{
  public GameDataDownloadException(string? message) : base(message) {}

  public GameDataDownloadException(string? message, Exception? innerException) : base(message, innerException) {}
}
