class TarsnapGui < Formula
  desc "Cross platform GUI for the Tarsnap command line client"
  homepage "https://github.com/Tarsnap/tarsnap-gui/wiki/Tarsnap"
  url "https://github.com/Tarsnap/tarsnap-gui/archive/v0.7.tar.gz"
  version "0.7"
  sha256 "fe81970589998b6f584b3630fd2c5bc270f5cf459e08572f23151c5a309d2e33"
  head "https://github.com/Tarsnap/tarsnap-gui.git"

  depends_on "qt5"
  depends_on "tarsnap"

  def install
    system "qmake"
    system "make -j$(sysctl -n hw.ncpu)"
    prefix.install 'Tarsnap.app'
  end

  test do
    system "#{opt_prefix}/Tarsnap.app/Contents/MacOS/Tarsnap", "--version"
  end
end
