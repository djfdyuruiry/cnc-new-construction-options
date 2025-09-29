using System;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;

namespace CNC.NCO.Launcher.Service;

public class MediaFireDownloader
{
  public async Task WithFileStream(string url, Action<Stream> onDownloadStart)
  {
    Console.WriteLine($"Parsing MediaFire file URL from webpage: {url}");
    
    // generate valid download link by requesting webpage html
    using var client = new HttpClient();
    client.Timeout = Timeout.InfiniteTimeSpan;

    using var htmlResponse = await client.GetAsync(url);
    htmlResponse.EnsureSuccessStatusCode();

    var output = await htmlResponse.Content.ReadAsStringAsync();

    var downloadHtml = output.Split("\n").FirstOrDefault(l => l.Contains("href=\"https://download")) ??
                       throw new Exception("Unable to parse download URL from MediaFire response");

    var resolvedDownloadUrl = downloadHtml.Split("\"").Length > 1
      ? downloadHtml.Split("\"")[1]
      : throw new Exception("Unable to parse download URL from MediaFire response");    

    Console.WriteLine($"Downloading MediaFire file URL: {resolvedDownloadUrl}");

    // download file using link
    using var response = await client.GetAsync(
      resolvedDownloadUrl,
      HttpCompletionOption.ResponseHeadersRead
    );
    
    response.EnsureSuccessStatusCode();
    await using var responseStream = await response.Content.ReadAsStreamAsync();
    
    onDownloadStart(responseStream);
  }
}
