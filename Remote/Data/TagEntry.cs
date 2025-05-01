using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MilkwaveRemote.Data {
  public class TagEntry {
    public TagEntry() { }

    public String PresetPath { get; set; } = "";
    
    public List<String> Tags { get; set; } =  new List<String>();
  }
}
