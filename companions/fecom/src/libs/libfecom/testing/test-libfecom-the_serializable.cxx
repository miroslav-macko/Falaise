/* test-libfecom-the_serializable.cxx
 *
 * Author(s)     : Guillaume Oliviero <goliviero@lpccaen.in2p3.fr>
 * Creation date : 2017-02-08
 * Last modified : 2017-02-08
 *
 * Copyright (C) 2017 Guillaume Oliviero <goliviero@lpccaen.in2p3.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Description :
 * This example illustrates the use of serializable
 * classes for the commissioning.
 *
 */

// Standard library:
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>

// Third party:
// - Boost:
// Code dedicated to the serialization of the ``std::set`` template class :
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>

// - Bayeux:
// - Bayeux/datatools:
#include <datatools/logger.h>
#include <datatools/io_factory.h>

// - Bayeux/brio:
#include <brio/writer.h>

// This project:
#include <fecom/commissioning_event.hpp>

void ex_com_event_1(datatools::logger::priority logging);
void ex_com_event_2(datatools::logger::priority logging);

int main (int argc_, char ** argv_)
{
  datatools::logger::priority logging = datatools::logger::PRIO_WARNING;
  try {

    int iarg = 1;
    while (iarg < argc_) {
      std::string token = argv_[iarg];
      if (token[0] == '-') {
        std::string option = token;
        if ((option == "-d") || (option == "--debug")) {
          logging = datatools::logger::PRIO_DEBUG;
        } else if ((option == "-v") || (option == "--verbose")) {
          logging = datatools::logger::PRIO_INFORMATION;
        } else  {
          DT_LOG_WARNING(logging, "Ignoring option '" << option << "' !");
        }
      } else {
        std::string argument = token;
        DT_LOG_WARNING(logging, "Ignoring argument '" << argument << "' !");
      }
      iarg++;
    }

    DT_LOG_INFORMATION(logging, "Welcome to the lib fecom example program : test-libfecom-the_serializable");

    // Serializer :
    ex_com_event_1(logging);

    // Deserializer
    ex_com_event_2(logging);

    DT_LOG_INFORMATION(logging, "The end.");

  } catch (std::exception & error) {
    std::cerr << "error: " << error.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/// Serialize a single ``commissioning_event`` instance in a txt file
void ex_com_event_1(datatools::logger::priority logging)
{
  DT_LOG_DEBUG(logging, "ENTERING EX_COM_EVENT_1");

  // Declare a ``commissioning_event`` object :
  DT_LOG_DEBUG(logging, "Declare a commissioning event");
  fecom::commissioning_event CE;
  uint32_t trigger_id = 12;
  CE.set_trigger_id(trigger_id);

  // Create a fake calo hit and some fake tracker channel hit to fill the commissioning event
  fecom::calo_hit chit;
  chit.hit_id = 42;
  chit.hitmode = fecom::base_hit::SIG_CALORIMETER;
  chit.slot_index = 0;
  chit.trigger_id = trigger_id;
  chit.channel = 11;
  chit.waveform_data_size = 16;
  for (std::size_t icell = 0; icell < chit.waveform_data_size; icell++) {
    chit.set_raw_sample(icell, 23);
  }
  chit.tree_dump(std::clog, "My Calo hit : ");

  DT_LOG_DEBUG(logging, "Add calo hit to commissioning event");
  CE.add_calo_hit(chit);


  for (std::size_t itchann = 0; itchann < 7; itchann++)
    {
      fecom::tracker_channel_hit tchit;
      tchit.hit_id = itchann;
      tchit.hitmode = fecom::base_hit::SIG_TRACKER;
      tchit.slot_index = 1;
      tchit.trigger_id = trigger_id;
      tchit.feast_id = 0;
      tchit.channel = 1;
      if (itchann < 5) tchit.channel_type = fecom::tracker_channel_hit::ANODIC_CHANNEL;
      else tchit.channel_type = fecom::tracker_channel_hit::CATHODIC_CHANNEL;
      std::string timestamp_type = "t" + std::to_string(itchann);
      tchit.timestamp_type = timestamp_type;
      tchit.timestamp_value = 42 * itchann;
      tchit.tree_dump(std::clog, "My tracker hit #" + std::to_string (itchann));

      DT_LOG_DEBUG(logging, "Add tracker channel hit to commissioning event");
      CE.add_tracker_channel_hit(tchit);
    }

  // Print it :
  DT_LOG_NOTICE(logging, "Commissioning event : ");
  CE.tree_dump(std::clog, "My commissioning event before serialization");

  std::string output_filename = "${FECOM_RESOURCES_DIR}/output_test/commissioning_event.data";
  datatools::fetch_path_with_env(output_filename);

  // Store the hit in a Boost/archive file (txt / XML format):
  DT_LOG_DEBUG(logging, "Serialize the commissioning event...");
  datatools::data_writer serializer(output_filename,
                                    datatools::using_multiple_archives);

  brio::writer my_writer("${FECOM_RESOURCES_DIR}/output_test/commissioning_event.brio", logging);
  my_writer.store(CE);

  serializer.store(CE);
  DT_LOG_INFORMATION(logging, "The hit has been stored in the ``commissioning_event.data`` file.");


  DT_LOG_DEBUG(logging, "EXITING EX_COM_EVENT_1");
  return;
}

/// Deserialize a single ``commissioning_event`` instance in a txt file
void ex_com_event_2(datatools::logger::priority logging)
{
  DT_LOG_DEBUG(logging, "ENTERING EX_COM_EVENT_2");
  std::string input_filename = "${FECOM_RESOURCES_DIR}/output_test/commissioning_event.data";
  datatools::fetch_path_with_env(input_filename);

  // Store the hit in a Boost/archive file (txt / XML format):
  DT_LOG_DEBUG(logging, "Serialize the commissioning event...");
  datatools::data_reader deserializer(input_filename,
				      datatools::using_multiple_archives);


  // Declare a ``commissioning_event`` object :
  DT_LOG_DEBUG(logging, "Declare an empty commissioning event");
  fecom::commissioning_event CE;

  deserializer.load(CE);
  DT_LOG_INFORMATION(logging, "The commissioning event has been loaded from the ``commissioning_event.data`` file.");

  CE.tree_dump(std::clog, "My commissioning event after deserialization");

  std::size_t calo_counter = 0;
  for (auto it_calo = CE.get_calo_hit_collection().begin();
       it_calo != CE.get_calo_hit_collection().end();
       it_calo++)
    {
      fecom::calo_hit a_calo_hit = * it_calo;
      a_calo_hit.tree_dump(std::clog, "Read from commissioning event calo #" + std::to_string(calo_counter));
      calo_counter++;
      std::clog << "calo counter = " << calo_counter << std::endl;
    }

  std::size_t tracker_counter = 0;
  for (auto it_tracker = CE.get_tracker_channel_hit_collection().begin();
       it_tracker != CE.get_tracker_channel_hit_collection().end();
       it_tracker++)
    {
      fecom::tracker_channel_hit a_tracker_channel_hit = * it_tracker;
      a_tracker_channel_hit.tree_dump(std::clog, "Read from commissioning event tracker #" + std::to_string(tracker_counter));
      tracker_counter++;
      std::clog << "tracker counter = " << tracker_counter << std::endl;
    }



  DT_LOG_DEBUG(logging, "EXITING EX_COM_EVENT_2");
  return;
}
