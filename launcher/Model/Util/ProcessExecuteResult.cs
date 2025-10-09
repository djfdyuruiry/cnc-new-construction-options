using System;
using System.Diagnostics;
using System.Linq;

namespace CNC.NCO.Launcher.Model.Util;

public class ProcessExecuteResult
{
  public ProcessExecuteResult(Process? process)
  {
    CreateProcessFailed = process is null;

    ExitCode = process?.ExitCode ?? -1;
    StdOut = process?.StandardOutput.ReadToEnd();
    StdErr = process?.StandardError.ReadToEnd();
  }

  public ProcessExecuteResult() : this(null) {}

  public int ExitCode { get; }
  public string? StdOut { get; }
  public string? StdErr { get; }
  public bool CreateProcessFailed { get; }

  public void AssertExitCode(string message, int exitCode = 0)
  {
    if (ExitCode != exitCode)
    {
      throw new Exception($"{message}\n{ToString()}");
    }
  }

  public override string ToString()
  {
    return string.Join(
      "\n",
      GetType().GetProperties().Select(p => $"{p.Name}: {p.GetValue(this)}")
    );
  }
}
