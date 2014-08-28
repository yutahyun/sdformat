/*
 * Copyright 2012-2014 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <sdf/sdf.hh>

namespace po = boost::program_options;

/////////////////////////////////////////////////
/// \brief Convert an SDF pose
/// \param[in] _elem SDF element to convert
/// \param[in] _prefix Prefix (spaces) used for formatting
/// \param[out] _result Stream that contains the conversion result
/// \return True on success
bool ConvertPose(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  // Output pose
  sdf::Pose pose = _elem->Get<sdf::Pose>();

  _result << _prefix << "<origin xyz='" << pose.pos << "' rpy='"
    << pose.rot << "'/>\n";
  return true;
}

/////////////////////////////////////////////////
/// \brief Convert an SDF geomentry
/// \param[in] _elem SDF element to convert
/// \param[in] _prefix Prefix (spaces) used for formatting
/// \param[out] _result Stream that contains the conversion result
/// \return True on success
bool ConvertGeometry(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  // Output <geometry> tag
  _result << _prefix << "<geometry>\n";

  // Output box
  if (_elem->HasElement("box"))
  {
    _result << _prefix << "  <box size='"
      << _elem->GetElement("box")->Get<sdf::Vector3>("size") << "'/>\n";
  }
  // Output sphere
  else if (_elem->HasElement("sphere"))
  {
    _result << _prefix << "  <sphere radius='"
      << _elem->GetElement("sphere")->Get<double>("radius") << "'/>\n";
  }
  // Output cylinder
  else if (_elem->HasElement("cylinder"))
  {
    _result << _prefix << "  <cylinder radius='"
      << _elem->GetElement("cylinder")->Get<double>("radius") << "' length='"
      << _elem->GetElement("cylinder")->Get<double>("length") << "'/>\n";
  }
  // Output mesh
  else if (_elem->HasElement("mesh"))
  {
    _result << _prefix << "  <mesh filename='"
      << _elem->GetElement("mesh")->Get<std::string>("uri") << "' scale='"
      << _elem->GetElement("mesh")->Get<sdf::Vector3>("scale") << "'/>\n";
  }

  // Output end <geometry> tag
  _result << _prefix << "</geometry>\n";

  return true;
}

/////////////////////////////////////////////////
/// \brief Convert an SDF collision
/// \param[in] _elem SDF element to convert
/// \param[in] _prefix Prefix (spaces) used for formatting
/// \param[out] _result Stream that contains the conversion result
/// \return True on success
bool ConvertCollision(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  // Output <collision>
  _result << _prefix << "<collision name='" << _elem->Get<std::string>("name")
          << "'>\n";

  // Output pose information
  ConvertPose(_elem->GetElement("pose"), _prefix + "  ", _result);

  // Convert geometry
  ConvertGeometry(_elem->GetElement("geometry"), _prefix + "  ", _result);

  // Output end <collision>
  _result << _prefix << "</collision>\n";

  return true;
}

/////////////////////////////////////////////////
bool ConvertVisual(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  // Output <visual>
  _result << _prefix << "<visual name='" << _elem->Get<std::string>("name")
          << "'>\n";

  // Output pose information
  ConvertPose(_elem->GetElement("pose"), _prefix + "  ", _result);

  // Convert geometry
  ConvertGeometry(_elem->GetElement("geometry"), _prefix + "  ", _result);

  _result << _prefix << "</visual>\n";
  return true;
}

/////////////////////////////////////////////////
bool ConvertInertia(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  // Output <inertial>
  _result << _prefix << "<inertial>\n";

  sdf::Pose pose = _elem->Get<sdf::Pose>("pose");

  // Output the pose
  _result << _prefix << "  <origin xyz='" << pose.pos << "' rpy='"
    << pose.rot << "' />\n";

  // Output the mass
  _result << _prefix << "  <mass>" << _elem->Get<double>("mass") << "</mass>\n";

  // Output the inertia
  sdf::ElementPtr inertia = _elem->GetElement("inertia");
  _result << _prefix << "  <inertia ixx='"
    << inertia->Get<double>("ixx") << "' ixy='"
    << inertia->Get<double>("ixy") << "' ixz='"
    << inertia->Get<double>("ixz") << "' iyy='"
    << inertia->Get<double>("iyy") << "' iyz='"
    << inertia->Get<double>("iyz") << "' izz='"
    << inertia->Get<double>("izz") << "'/>\n";

  // Output end <inertial>
  _result << _prefix << "</inertial>\n";

  return true;
}

/////////////////////////////////////////////////
bool ConvertLink(sdf::ElementPtr _elem, const std::string &_prefix,
    std::map<std::string, sdf::Pose> &_linkPoses,
    std::ostringstream &_result)
{
  // Output <link>
  _result << _prefix << "<link name='" << _elem->Get<std::string>("name")
          << "'>\n";

  // Store the link pose
  _linkPoses[_elem->Get<std::string>("name")] = _elem->Get<sdf::Pose>("pose");

  // Output inertia information
  ConvertInertia(_elem->GetElement("inertial"), _prefix + "  ", _result);

  // Convert all collisions
  if (_elem->HasElement("collision"))
  {
    sdf::ElementPtr collisionElem = _elem->GetElement("collision");
    while (collisionElem)
    {
      ConvertCollision(collisionElem, _prefix + "  ", _result);
      collisionElem = collisionElem->GetNextElement("collision");
    }
  }

  // Convert all visuals
  if (_elem->HasElement("visual"))
  {
    sdf::ElementPtr visualElem = _elem->GetElement("visual");
    while (visualElem)
    {
      ConvertVisual(visualElem, _prefix + "  ", _result);
      visualElem = visualElem->GetNextElement("visual");
    }
  }

  // Output end <link>
  _result << _prefix << "</link>\n";
  return true;
}

/////////////////////////////////////////////////
bool ConvertCamera(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  if (!_elem->HasElement("image") || !_elem->HasElement("clip"))
    return false;

  // Output <camera>
  _result << _prefix << "<camera>\n";

  sdf::ElementPtr imgElem = _elem->GetElement("image");
  sdf::ElementPtr clipElem = _elem->GetElement("clip");

  _result << _prefix << "  <image "
    << "width='" << imgElem->Get<int>("width") << "' "
    << "height='" << imgElem->Get<int>("height") << "' "
    << "format='" << imgElem->Get<std::string>("format") << "' "
    << "hfov='" << _elem->Get<double>("horizontal_fov") << "' "
    << "near='" << clipElem->Get<double>("near") << "' "
    << "far='" << clipElem->Get<double>("far") << "' "
    << "/>\n";

  // Output </camera>
  _result << _prefix << "</camera>\n";
  return true;
}

/////////////////////////////////////////////////
bool ConvertRay(sdf::ElementPtr _elem, const std::string &_prefix,
    std::ostringstream &_result)
{
  if (!_elem->HasElement("scan"))
    return false;

  sdf::ElementPtr scanElem = _elem->GetElement("scan");

  // Output <ray>
  _result << _prefix << "<ray>\n";

  if (scanElem->HasElement("horizontal"))
  {
    sdf::ElementPtr horzElem = scanElem->GetElement("horizontal");
    _result << _prefix << "  <horizontal "
      << "samples='" << horzElem->Get<unsigned int>("samples") << "' "
      << "resolution='" << horzElem->Get<double>("resolution") << "' "
      << "min_angle='" << horzElem->Get<double>("min_angle") << "' "
      << "max_angle='" << horzElem->Get<double>("max_angle") << "' "
      << "/>\n";
  }

  if (scanElem->HasElement("vertical"))
  {
    sdf::ElementPtr vertElem = scanElem->GetElement("vertical");
    _result << _prefix << "  <vertical "
      << "samples='" << vertElem->Get<unsigned int>("samples") << "' "
      << "resolution='" << vertElem->Get<double>("resolution") << "' "
      << "min_angle='" << vertElem->Get<double>("min_angle") << "' "
      << "max_angle='" << vertElem->Get<double>("max_angle") << "' "
      << "/>\n";
  }

  _result << _prefix << "</ray>\n";
  return true;
}

/////////////////////////////////////////////////
bool ConvertSensor(sdf::ElementPtr _elem, const std::string &_prefix,
    const std::string &_parent, std::ostringstream &_result)
{
  // Output <sensor>
  _result << _prefix << "<sensor name='" << _elem->Get<std::string>("name")
    << "' update_rate='" << _elem->Get<double>("update_rate") << "'>\n";

  _result << _prefix << "  <parent link='" << _parent << "'/>\n";
  ConvertPose(_elem->GetElement("pose"), _prefix + "  ", _result);

  if (_elem->HasElement("camera"))
    ConvertCamera(_elem->GetElement("camera"), _prefix + "  ", _result);
  else if (_elem->HasElement("ray"))
    ConvertRay(_elem->GetElement("ray"), _prefix + "  ", _result);

  // Output </sensor>
  _result << _prefix << "</sensor>\n";

  return true;
}

/////////////////////////////////////////////////
bool ConvertJoint(sdf::ElementPtr _elem, const std::string &_prefix,
    std::map<std::string, sdf::Pose> &_linkPoses,
    std::ostringstream &_result)
{
  // Output <joint>
  _result << _prefix << "<joint name='" << _elem->Get<std::string>("name")
    << "' type='" << _elem->Get<std::string>("type") << "'>\n";

  std::string childLink = _elem->Get<std::string>("child");
  std::string parentLink = _elem->Get<std::string>("parent");

  sdf::Pose childPose = _linkPoses[childLink];
  sdf::Pose parentPose = _linkPoses[parentLink];
  sdf::Pose jointPose = _elem->Get<sdf::Pose>("pose");

  jointPose = (jointPose + childPose) - parentPose;

  // Output pose
  _result << _prefix << "  <origin xyz='" << jointPose.pos << "' rpy='"
    << jointPose.rot << "'/>\n";

  // Output parent link
  _result << _prefix << "  <parent link='" << parentLink << "'/>\n";

  // Output child link
  _result << _prefix << "  <child link='"
    <<  childLink << "'/>\n";

  sdf::ElementPtr axisElem = _elem->GetElement("axis");

  // Output dynamics
  if (axisElem->HasElement("dynamics"))
  {
    sdf::ElementPtr dynamicsElem = axisElem->GetElement("dynamics");
    _result << _prefix << "  <dynamics damping='"
      << dynamicsElem->Get<double>("damping") << "' friction='"
      << dynamicsElem->Get<double>("friction") << "' />\n";
  }

  // Output axis
  _result << _prefix << "  <axis xyz='"
    << axisElem->Get<sdf::Vector3>("xyz") << "'/>\n";

  // Output limits
  sdf::ElementPtr limitElem = axisElem->GetElement("limit");
  _result << _prefix << "  <limit lower='" << limitElem->Get<double>("lower")
    << "' upper='" << limitElem->Get<double>("upper")
    << "' effort='" << limitElem->Get<double>("effort") << "' velocity='"
    << limitElem->Get<double>("velocity") << "'/>\n";

  // Output </joint>
  _result << _prefix << "</joint>\n";

  return true;
}

/////////////////////////////////////////////////
bool ConvertModel(sdf::ElementPtr _elem, std::ostringstream &_result)
{
  // Outut the <robot> tag
  _result << "<robot name='" << _elem->Get<std::string>("name") << "'>\n";

  std::map<std::string, sdf::Pose> linkPoses;

  // Convert all links
  if (_elem->HasElement("link"))
  {
    sdf::ElementPtr linkElem = _elem->GetElement("link");
    while (linkElem)
    {
      ConvertLink(linkElem, "  ", linkPoses, _result);

      // Convert all sensors attached to the link
      if (linkElem->HasElement("sensor"))
      {
        sdf::ElementPtr sensorElem = linkElem->GetElement("sensor");
        while (sensorElem)
        {
          ConvertSensor(sensorElem, "  ", linkElem->Get<std::string>("name"),
                        _result);
          sensorElem = sensorElem->GetNextElement("sensor");
        }
      }

      linkElem = linkElem->GetNextElement("link");
    }
  }

  // Convert all joints
  if (_elem->HasElement("joint"))
  {
    sdf::ElementPtr jointElem = _elem->GetElement("joint");
    while (jointElem)
    {
      ConvertJoint(jointElem, "  ", linkPoses, _result);
      jointElem = jointElem->GetNextElement("joint");
    }
  }


  // Output the end <robot> tag
  _result << "</robot>\n";
  return true;
}

/////////////////////////////////////////////////
bool Convert(const std::string &_file, std::ostringstream &_result)
{
  // Make sure the file exists
  boost::filesystem::path path = _file;
  if (!boost::filesystem::exists(path))
  {
    std::cerr << "Error: File doesn't exist[" << path.string() << "]\n";
    return false;
  }

  // Create and initialize SDF
  boost::shared_ptr<sdf::SDF> sdf(new sdf::SDF());
  if (!sdf::init(sdf))
  {
    std::cerr << "ERROR: SDF parsing the xml failed" << std::endl;
    return false;
  }

  // Read the SDF file
  if (!sdf::readFile(path.string(), sdf))
  {
    std::cerr << "Error: SDF parsing the xml failed\n";
    return false;
  }

  // Read the model
  sdf::ElementPtr modelElem = sdf->root->GetElement("model");

  // Output xml encoding
  _result << "<?xml version='1.0' encoding='utf-8'?>\n";

  // Output the model as URDF
  if (modelElem)
  {
    ConvertModel(modelElem, _result);
  }

  return true;
}

/////////////////////////////////////////////////
int main(int _argc, char **_argv)
{
  po::variables_map vm;
  po::options_description visibleOptions;
  visibleOptions.add_options()
    ("help,h", "Print this help message")
    ("file,f", po::value<std::string>(), "SDF file to convert to URDF")
    ("out,o", po::value<std::string>(), "Output filename");

  // Parse the command line arguments
  try
  {
    po::store(
        po::command_line_parser(_argc, _argv).options(
          visibleOptions).run(), vm);
    po::notify(vm);
  }
  catch(boost::exception &_e)
  {
    std::cerr << "Invalid arguments\n";
    return -1;
  }

  // Convert the given file.
  if (vm.count("file"))
  {
    std::ostringstream result;
    if (!Convert(vm["file"].as<std::string>(), result))
    {
      std::cerr << "Unable to convert file[" << vm["file"].as<std::string>()
        << std::endl;
      return -1;
    }

    // Output to file, if specified. Otherwise output to screen
    if (vm.count("out"))
    {
      std::ofstream out(vm["out"].as<std::string>().c_str(), std::ios::out);
      out << result.str();
      out.close();
    }
    else
    {
      std::cout << result.str();
    }
  }
  else
  {
    std::cerr << "Specify a file to convert with the -f option\n";
    return -1;
  }

  return 0;
}