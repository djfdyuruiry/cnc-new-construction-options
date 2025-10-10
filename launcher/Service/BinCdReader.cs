using System.IO;
using DiscUtils.Iso9660;

namespace CNC.NCO.Launcher.Service;

public class BinCdReader(Stream data) : CDReader(data, true)
{
  protected override void Dispose(bool disposing)
  {
    base.Dispose(disposing);

    
  }
}
