#include <Exceptions.h>
#include <ImageIO.h>
#include <ImagesCPU.h>
#include <ImagesNPP.h>

#include <string.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <cuda_runtime.h>
#include <npp.h>

#include <helper_cuda.h>
#include <helper_string.h>

bool printfNPPinfo(int argc, char *argv[])
{
  const NppLibraryVersion *libVer = nppGetLibVersion();

  printf("NPP Library Version %d.%d.%d\n", libVer->major, libVer->minor,
         libVer->build);

  int driverVersion, runtimeVersion;
  cudaDriverGetVersion(&driverVersion);
  cudaRuntimeGetVersion(&runtimeVersion);

  printf("  CUDA Driver  Version: %d.%d\n", driverVersion / 1000,
         (driverVersion % 100) / 10);
  printf("  CUDA Runtime Version: %d.%d\n", runtimeVersion / 1000,
         (runtimeVersion % 100) / 10);

  // Min spec is SM 1.0 devices
  bool bVal = checkCudaCapabilities(1, 0);
  return bVal;
}

int main(int argc, char *argv[])
{
  printf("%s Starting...\n\n", argv[0]);

  try
  {
    std::string sPath;
    char *path;

    findCudaDevice(argc, (const char **)argv);

    if (printfNPPinfo(argc, argv) == false)
    {
      exit(EXIT_SUCCESS);
    }

    if (checkCmdLineFlag(argc, (const char **)argv, "input"))
    {
      getCmdLineArgumentString(argc, (const char **)argv, "input", &path);
    }

    if (path)
    {
      sPath = path;
    }
    else
    {
      sPath = "data";
    }

    std::string output_dir = "./output/";

    if (!std::filesystem::is_directory(output_dir) || !std::filesystem::exists(output_dir))
    {
      std::filesystem::create_directory(output_dir);
    }

    for (auto const &entry : std::filesystem::directory_iterator(sPath))
    {
      std::string sFile = entry.path();
      // if we specify the filename at the command line, then we only test
      // sFile[0].
      int file_errors = 0;
      std::ifstream infile(sFile.data(), std::ifstream::in);

      if (infile.good())
      {
        std::cout << "boxFilterNPP opened: <" << sFile.data()
                  << "> successfully!" << std::endl;
        file_errors = 0;
        infile.close();
      }
      else
      {
        std::cout << "boxFilterNPP unable to open: <" << sFile.data() << ">"
                  << std::endl;
        file_errors++;
        infile.close();
      }

      if (file_errors > 0)
      {
        exit(EXIT_FAILURE);
      }

      std::string sResultFilename = sFile;

      std::string::size_type dot = sResultFilename.rfind('.');

      if (dot != std::string::npos)
      {
        sResultFilename = sResultFilename.substr(0, dot);
      }

      std::string::size_type sep = sResultFilename.rfind('/');

      if (sep != std::string::npos)
      {
        sResultFilename = sResultFilename.substr(sep + 1);
      }

      sResultFilename = output_dir + sResultFilename + "_boxFilter.jpg";

      if (checkCmdLineFlag(argc, (const char **)argv, "output"))
      {
        char *outputFilePath;
        getCmdLineArgumentString(argc, (const char **)argv, "output",
                                 &outputFilePath);
        sResultFilename = outputFilePath;
      }

      // declare a host image object for an 8-bit grayscale image
      npp::ImageCPU_8u_C1 oHostSrc;
      // load gray-scale image from disk
      npp::loadImage(sFile, oHostSrc);
      // declare a device image and copy construct from the host image,
      // i.e. upload host to device
      npp::ImageNPP_8u_C1 oDeviceSrc(oHostSrc);

      // create struct with box-filter mask size
      NppiSize oMaskSize = {5, 5};

      NppiSize oSrcSize = {(int)oDeviceSrc.width(), (int)oDeviceSrc.height()};
      NppiPoint oSrcOffset = {0, 0};

      // create struct with ROI size
      NppiSize oSizeROI = {(int)oDeviceSrc.width(), (int)oDeviceSrc.height()};
      // allocate device image of appropriately reduced size
      npp::ImageNPP_8u_C1 oDeviceDst(oSizeROI.width, oSizeROI.height);
      // set anchor point inside the mask to (oMaskSize.width / 2,
      // oMaskSize.height / 2) It should round down when odd
      NppiPoint oAnchor = {oMaskSize.width / 2, oMaskSize.height / 2};

      // run box filter
      NPP_CHECK_NPP(nppiFilterBoxBorder_8u_C1R(
          oDeviceSrc.data(), oDeviceSrc.pitch(), oSrcSize, oSrcOffset,
          oDeviceDst.data(), oDeviceDst.pitch(), oSizeROI, oMaskSize, oAnchor,
          NPP_BORDER_REPLICATE));

      // declare a host image for the result
      npp::ImageCPU_8u_C1 oHostDst(oDeviceDst.size());
      // and copy the device result data into it
      oDeviceDst.copyTo(oHostDst.data(), oHostDst.pitch());

      saveImage(sResultFilename, oHostDst);
      std::cout << "Saved image: " << sResultFilename << std::endl;
    }
  }
  catch (npp::Exception &rException)
  {
    std::cerr << "Program error! The following exception occurred: \n";
    std::cerr << rException << std::endl;
    std::cerr << "Aborting." << std::endl;

    exit(EXIT_FAILURE);
  }
  catch (...)
  {
    std::cerr << "Program error! An unknow type of exception occurred. \n";
    std::cerr << "Aborting." << std::endl;

    exit(EXIT_FAILURE);
    return -1;
  }
  exit(EXIT_SUCCESS);

  return 0;
}
