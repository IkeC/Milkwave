using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MilkwaveRemote.Data {
  public class Tags {
    public Tags() {
    }
    public Dictionary<string, TagEntry> TagEntries { get; set; } = new Dictionary<string, TagEntry>();
  }
}
