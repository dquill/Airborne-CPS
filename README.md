# Airborne-CPS
A Traffic Collusion Avoidance System Mark II (TCAS-II) implementation for Laminar Research X-Plane

### Current Functionality

- Threat detection within a protection volume.
- Threat classifications:
  * Traffic Advisory(TA)
  * Resolution Advisory(RA)
- RA action consensus and action suggestion.
- Drawing of recommended action(vertical velocity) to gauge.
- Suport for at least 3 client connections.
- Strength-Based RA Selection Algorithm for 3+ aircraft RA scenarios.

### Description / Setup
This project is a plugin for the Laminar Research X-Plane flight simulator, and is an implementation of a [Traffic Collision Avoidance System](https://www.faa.gov/documentLibrary/media/Advisory_Circular/TCAS%20II%20V7.1%20Intro%20booklet.pdf), mark II. It is used in Cyber Physical Systems research at the State University of New York at Oswego; please see "Literature" for examples and related work.

- The plugin was written in C++ in Microsoft Visual Studio Community 2019.

- The SDK for the plugin is included in the codebase, and can also be found [here](http://www.xsquawkbox.net/xpsdk/mediawiki/Main_Page).

- Install the Github extension for Visual Studio: https://marketplace.visualstudio.com/items?itemName=GitHub.GitHubExtensionforVisualStudio

- Open Visual Studio 2019 and select Clone or Checkout Code from the Get Started wizard

- Get the web URL from the github page (green button, copy the URL it provides)

- Select a location for your local copy, or accept the default that VS provides, and proceed to clone

- Open Airbone-CPS.sln in the cloned repo (view is be default on the right side)

- Set project to Release and x86, then build. Should build without problems. Compiled output will be found at [local_repo_path]\out\x86\AirborneCPS.pdg and .xpl
    - these files need to go into X-Plane 10\Resources\plugins\ folder each time you build. 

- Copy the included Images folder located in Specification to X-Plane 10\Resources\plugins and rename the Images folder to AirborneCPS
- Copy the included situations folder located in Specification and replace the situation folder located at X-Plane 10\Output



- Build plugin
    * Use Release and x86 currently.
