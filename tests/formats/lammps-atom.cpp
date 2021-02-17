// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

#include "catch.hpp"
#include "chemfiles.hpp"
#include "helpers.hpp"
using namespace chemfiles;

// {wrapped, scaled_wrapped, unwrapped, scaled_unwrapped}.lammpstrj
// are based on the same simulation and contain therefore the same unwrapped positions
static void check_pos_representation(Trajectory& file) {
    CHECK(file.nsteps() == 11);

    Frame frame = file.read();
    CHECK(frame.size() == 7751);

    CHECK(frame.cell().shape() == UnitCell::ORTHORHOMBIC);
    CHECK(approx_eq(frame.cell().lengths(), Vector3D(35.7, 35.7, 92.82), 1e-2));
    CHECK(approx_eq(frame.cell().angles(), Vector3D(90.0, 90.0, 90.0), 1e-2));

    auto positions = frame.positions();
    CHECK(approx_eq(positions[5000], Vector3D(12.2614, 7.76219, -13.0444), 1e-3));
    CHECK(approx_eq(positions[7000], Vector3D(15.7755, 15.7059, 20.9502), 1e-3));

    auto velocities = *frame.velocities();
    CHECK(approx_eq(velocities[5000], Vector3D(-0.000273223, 0.000143908, -0.000557713), 1e-7));
    CHECK(approx_eq(velocities[7000], Vector3D(-0.000466344, 0.000701151, 0.000430329), 1e-7));

    CHECK(approx_eq(frame[5000].charge(), 0.5564));
    CHECK(frame[5000].type() == "2");
    CHECK(frame[5000].name() == "C");

    frame = file.read_step(5);
    CHECK(frame.size() == 7751);

    positions = frame.positions();
    CHECK(approx_eq(positions[5000], Vector3D(4.33048, 4.23699, -2.29954), 1e-3));
    CHECK(approx_eq(positions[7000], Vector3D(15.9819, 21.1517, 8.12739), 1e-3));

    velocities = *frame.velocities();
    CHECK(approx_eq(velocities[5000], Vector3D(-0.00404259, -0.000939097, 0.0152453), 1e-7));
    CHECK(approx_eq(velocities[7000], Vector3D(0.00122365, 0.0100476, -0.0167459), 1e-7));

    CHECK_THROWS_AS(file.read_step(11), FileError);
}

TEST_CASE("Read files in LAMMPS Atom format") {
    SECTION("Polymer") {
        auto file = Trajectory("data/lammps/polymer.lammpstrj");
        Frame frame = file.read();
        double eps = 1e-3;

        CHECK(frame.size() == 1714);
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(51.8474, 100.348, 116.516), eps));
        // this one has a non zero image index (1 0 0)
        CHECK(approx_eq(positions[1189], Vector3D(116.829, 91.2404, 79.8858), eps));
        // this one has a non zero image index (2 1 -3)
        CHECK(approx_eq(positions[1327], Vector3D(173.311, 87.853, 109.417), eps));
    }

    SECTION("NaCl") {
        auto file = Trajectory("data/lammps/nacl.lammpstrj");
        Frame frame = file.read();

        CHECK(frame.size() == 512);
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(0.0, 0.0, 0.0), 1e-3));
        CHECK(approx_eq(positions[222], Vector3D(14.1005, 0.0, 8.4603), 1e-3));

        auto velocities = *frame.velocities();
        CHECK(approx_eq(velocities[0], Vector3D(-0.00258494, 0.00270859, -0.00314039), 1e-7));
        CHECK(approx_eq(velocities[222], Vector3D(-0.00466812, -0.00196397, -0.000147051), 1e-7));

        frame = file.read_step(5);
        CHECK(frame.size() == 512);
        positions = frame.positions();

        CHECK(approx_eq(positions[0], Vector3D(0.095924, -0.0222584, -0.0152489), 1e-3));
        CHECK(approx_eq(positions[222], Vector3D(14.0788, 0.0954186, 8.56453), 1e-3));

        frame = file.read_step(0); // read a previous step
        CHECK(frame.size() == 512);
        positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(0.0, 0.0, 0.0), 1e-3));
        CHECK(approx_eq(positions[222], Vector3D(14.1005, 0.0, 8.4603), 1e-3));

        CHECK_THROWS_AS(file.read_step(6), FileError);
    }

    SECTION("Wrapped Coordinates") {
        auto file = Trajectory("data/lammps/wrapped.lammpstrj");
        check_pos_representation(file);
    }

    SECTION("Scaled Wrapped Coordinates") {
        auto file = Trajectory("data/lammps/scaled_wrapped.lammpstrj");
        check_pos_representation(file);
    }

    SECTION("Unrapped Coordinates") {
        auto file = Trajectory("data/lammps/unwrapped.lammpstrj");
        check_pos_representation(file);
    }

    SECTION("Scaled Unrapped Coordinates") {
        auto file = Trajectory("data/lammps/scaled_unwrapped.lammpstrj");
        check_pos_representation(file);
    }

    SECTION("Position Representation") {
        auto file = Trajectory("data/lammps/detect_best_pos_repr.lammpstrj");
        CHECK(file.nsteps() == 5);

        Frame frame = file.read();
        CHECK(frame.size() == 854);
        CHECK(frame.step() == 100000);
        CHECK(approx_eq((*frame.get("time")).as_double(), 25e9, 1e-6));
        auto positions = frame.positions();

        CHECK(approx_eq(positions[679], Vector3D(1.47679, -25.2886, 2.38234), 1e-3));
        CHECK(approx_eq(positions[764], Vector3D(-256.58, 117.368, 1.9654), 1e-3));

        frame = file.read();
        CHECK(frame.size() == 854);
        CHECK(frame.step() == 101000);
        CHECK(!frame.get("time"));
        positions = frame.positions();

        CHECK(frame.cell().shape() == UnitCell::ORTHORHOMBIC);
        CHECK(approx_eq(frame.cell().lengths(), Vector3D(60.0, 60.0, 250.0), 1e-2));
        CHECK(approx_eq(frame.cell().angles(), Vector3D(90.0, 90.0, 90.0), 1e-2));

        CHECK(approx_eq(positions[683], Vector3D(-43.3683, 322.948, 208.063), 1e-3));
        CHECK(approx_eq(positions[828], Vector3D(150.083, -135.113, 189.641), 1e-3));

        frame = file.read();
        CHECK(frame.size() == 856);
        CHECK(frame.step() == 102000);

        positions = frame.positions();
        CHECK(approx_eq(positions[747], Vector3D(-158.317, 142.593, 2.11392), 1e-3));
        CHECK(approx_eq(positions[799], Vector3D(224.784, -167.878, 39.3765), 1e-3));

        frame = file.read();
        CHECK(frame.size() == 856);
        CHECK(frame.step() == 103000);

        positions = frame.positions();
        CHECK(approx_eq(positions[735], Vector3D(67.2657, 30.0627, 2.1141), 1e-3));
        CHECK(approx_eq(positions[775], Vector3D(125.347, -82.3507, 46.611), 1e-3));

        frame = file.read();
        CHECK(frame.size() == 856);
        CHECK(frame.step() == 104000);

        positions = frame.positions();
        CHECK(approx_eq(positions[652], Vector3D(-188.131, 96.0777, 196.23), 1e-3));
        CHECK(approx_eq(positions[838], Vector3D(-33.6068, -50.5113, 209.306), 1e-3));

        CHECK_THROWS_AS(file.read(), FileError);
    }

    SECTION("Errors") {
        auto file = Trajectory("data/lammps/broken.lammpstrj");
        std::string msg = "can not read box header in LAMMPS format: expected an ITEM entry in "
                          "LAMMPS format, got 'DUMMY'";
        CHECK_THROWS_WITH(file.read_step(0), msg);
        msg =
            "can not read box header in LAMMPS format: missing 'BOX BOUNDS' item in LAMMPS format";
        CHECK_THROWS_WITH(file.read_step(1), msg);
        msg = "can not read box header in LAMMPS format: incomplete box dimensions in LAMMPS "
              "format, expected 2 but got 1";
        CHECK_THROWS_WITH(file.read_step(2), msg);
        CHECK_THROWS_WITH(file.read_step(3), msg);
        CHECK_THROWS_WITH(file.read_step(4), msg);
        msg = "can not read box header in LAMMPS format: incomplete box dimensions in LAMMPS "
              "format, expected 3 but got 2";
        CHECK_THROWS_WITH(file.read_step(5), msg);
        CHECK_THROWS_WITH(file.read_step(6), msg);
        CHECK_THROWS_WITH(file.read_step(7), msg);
        msg = "can not read next step as LAMMPS format: expected an ITEM entry";
        CHECK_THROWS_WITH(file.read_step(8), msg);
        CHECK_THROWS_WITH(file.read_step(9), msg);
        CHECK_THROWS_WITH(file.read_step(10), msg);
        CHECK_THROWS_WITH(file.read_step(12), msg);
        msg = "can not read next step as LAMMPS format: expected 'TIMESTEP' got 'DUMMY'";
        CHECK_THROWS_WITH(file.read_step(11), msg);
        msg = "can not read next step as LAMMPS format: expected 'ATOMS' got 'DUMMY'";
        CHECK_THROWS_WITH(file.read_step(13), msg);
        msg = "LAMMPS line has wrong number of fields: expected 5 got 6";
        CHECK_THROWS_WITH(file.read_step(14), msg);
        msg = "found atoms with the same ID in LAMMPS format: 2 is already present";
        CHECK_THROWS_WITH(file.read_step(15), msg);
        CHECK_THROWS_AS(file.read_step(16), FileError);
    }
}

TEST_CASE("Read files in memory") {
    SECTION("Reading from memory") {
        std::string content(R"(ITEM: TIMESTEP
0
ITEM: NUMBER OF ATOMS
2
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 2.0000000000000000e+01
0.0000000000000000e+00 3.0000000000000000e+01
0.0000000000000000e+00 4.0000000000000000e+01
ITEM: ATOMS id type x y z
1 1 5 5 5
2 5 6.5 6.5 6.5
)");

        auto file = Trajectory::memory_reader(content.data(), content.size(), "LAMMPS");
        CHECK(file.nsteps() == 1);

        auto frame = file.read();
        CHECK(frame.size() == 2);
        CHECK(frame.cell().shape() == UnitCell::ORTHORHOMBIC);
        CHECK(approx_eq(frame.cell().lengths(), Vector3D(20.0, 30.0, 40.0), 1e-2));
        CHECK(approx_eq(frame.cell().angles(), Vector3D(90.0, 90.0, 90.0), 1e-2));
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(5.0, 5.0, 5.0), 1e-2));
        CHECK(approx_eq(positions[1], Vector3D(6.5, 6.5, 6.5), 1e-2));
        CHECK(frame[0].type() == "1");
        CHECK(frame[0].name() == "");
        CHECK(frame[1].type() == "5");
    }

    SECTION("Frame properties") {
        std::string content(R"(ITEM: UNITS
lj
ITEM: TIME
250.5
ITEM: TIMESTEP
5
ITEM: NUMBER OF ATOMS
0
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
ITEM: ATOMS id type x y z
ITEM: UNITS
metal
ITEM: TIMESTEP
15
ITEM: NUMBER OF ATOMS
3
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
ITEM: ATOMS id type x y z
1 1 5 5 5
2 1 5 5 5
3 1 5 5 5
ITEM: TIME
335.678
ITEM: TIMESTEP
20
ITEM: NUMBER OF ATOMS
0
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
0.0000000000000000e+00 1.0000000000000000e+01
ITEM: ATOMS id type x y z
)");

        auto file = Trajectory::memory_reader(content.data(), content.size(), "LAMMPS");
        CHECK(file.nsteps() == 3);

        auto frame = file.read();
        CHECK(frame.size() == 0);
        CHECK(*frame.get("lammps_units") == "lj");
        CHECK(approx_eq((*frame.get("time")).as_double(), 250.5, 1e-6));
        CHECK(frame.step() == 5);

        frame = file.read();
        CHECK(frame.size() == 3);
        CHECK(*frame.get("lammps_units") == "metal");
        CHECK(!frame.get("time"));
        CHECK(frame.step() == 15);

        frame = file.read();
        CHECK(frame.size() == 0);
        CHECK(!frame.get("lammps_units"));
        CHECK(approx_eq((*frame.get("time")).as_double(), 335.678, 1e-6));
        CHECK(frame.step() == 20);
    }

    SECTION("Atom properties") {
        std::string content(R"(ITEM: TIMESTEP
7
ITEM: NUMBER OF ATOMS
2
ITEM: BOX BOUNDS pp pp pp
-1.5000000000000000e+00 2.0000000000000000e+01
-2.6000000000000000e+00 3.0000000000000000e+01
-3.7000000000000000e+00 4.0000000000000000e+01
ITEM: ATOMS type element z mass y x vy vz q id
32 Ge -1.234 72.6 50.432 1.555 -2.345 6.456 2.5 2
87 Fr 7 223.0 6 5 8 9 -1 1
)"); // column order very messed up

        auto file = Trajectory::memory_reader(content.data(), content.size(), "LAMMPS");
        CHECK(file.nsteps() == 1);

        auto frame = file.read();
        CHECK(frame.size() == 2);
        CHECK(approx_eq(frame.cell().lengths(), Vector3D(21.5, 32.6, 43.7), 1e-2));
        CHECK(approx_eq(frame.cell().angles(), Vector3D(90.0, 90.0, 90.0), 1e-2));
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(5.0, 6.0, 7.0), 1e-2));
        CHECK(approx_eq(positions[1], Vector3D(1.555, 50.432, -1.234), 1e-2));
        auto velocities = *frame.velocities();
        CHECK(approx_eq(velocities[0], Vector3D(0.0, 8.0, 9.0), 1e-6));
        CHECK(approx_eq(velocities[1], Vector3D(0.0, -2.345, 6.456), 1e-6));
        CHECK(frame.step() == 7);
        CHECK(frame[0].type() == "87");
        CHECK(frame[1].type() == "32");
        CHECK(frame[0].name() == "Fr");
        CHECK(frame[1].name() == "Ge");
        CHECK(approx_eq(frame[0].mass(), 223.0, 1e-6));
        CHECK(approx_eq(frame[1].mass(), 72.6, 1e-6));
        CHECK(approx_eq(frame[0].charge(), -1, 1e-6));
        CHECK(approx_eq(frame[1].charge(), 2.5, 1e-6));
    }

    SECTION("Best position representation") {
        std::string content(R"(ITEM: TIMESTEP
0
ITEM: NUMBER OF ATOMS
1
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 2.0000000000000000e+01
0.0000000000000000e+00 3.0000000000000000e+01
0.0000000000000000e+00 4.0000000000000000e+01
ITEM: ATOMS id type y z xs ys zs
1 1 -1 -1 0.5 0.5 0.5
ITEM: TIMESTEP
1
ITEM: NUMBER OF ATOMS
1
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 2.0000000000000000e+01
0.0000000000000000e+00 3.0000000000000000e+01
0.0000000000000000e+00 4.0000000000000000e+01
ITEM: ATOMS id type x y z xu yu zu xus yus zus
1 1 -1 -1 -1 150.5 160.6 170.7 -1 -1 -1
ITEM: TIMESTEP
2
ITEM: NUMBER OF ATOMS
1
ITEM: BOX BOUNDS pp pp pp
0.0000000000000000e+00 2.0000000000000000e+01
0.0000000000000000e+00 3.0000000000000000e+01
0.0000000000000000e+00 4.0000000000000000e+01
ITEM: ATOMS id type
1 1
)");

        auto file = Trajectory::memory_reader(content.data(), content.size(), "LAMMPS");
        CHECK(file.nsteps() == 3);

        auto frame = file.read();
        CHECK(frame.size() == 1);
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(10.0, 15.0, 20.0), 1e-2));

        frame = file.read();
        CHECK(frame.size() == 1);
        positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(150.5, 160.6, 170.7), 1e-2));

        frame = file.read();
        CHECK(frame.size() == 1);
        positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(0.0, 0.0, 0.0), 1e-2));
    }

    SECTION("Triclinic boxes") {
        std::string content(R"(ITEM: TIMESTEP
0
ITEM: NUMBER OF ATOMS
1
ITEM: BOX BOUNDS pp pp pp xy xz yz
-4.0000000000000000e+00 6.0000000000000000e+00 5.0000000000000000e+00
0.0000000000000000e+00 2.0000000000000000e+01 4.0000000000000000e+00
-1.0000000000000000e+00 1.0000000000000000e+01 3.5000000000000000e+00
ITEM: ATOMS id type x y z
1 1 5 5 5
ITEM: TIMESTEP
1
ITEM: NUMBER OF ATOMS
1
ITEM: BOX BOUNDS xy xz yz pp pp pp
-4.0000000000000000e+00 6.0000000000000000e+00 5.0000000000000000e+00
0.0000000000000000e+00 2.0000000000000000e+01 4.0000000000000000e+00
-1.0000000000000000e+00 1.0000000000000000e+01 3.5000000000000000e+00
ITEM: ATOMS id type xs ys zs ix iy iz
1 1 0.604545 0.154545 0.545455 3 1 1
)"); // in older LAMMPS versions (pre Apr 2011 [f7ce527]) the boundary flags come before 'xy xz yz'

        auto file = Trajectory::memory_reader(content.data(), content.size(), "LAMMPS");
        CHECK(file.nsteps() == 2);

        auto frame = file.read();
        CHECK(frame.size() == 1);
        CHECK(frame.cell().shape() == UnitCell::TRICLINIC);
        CHECK(approx_eq(frame.cell().lengths(), Vector3D(10.0, 20.616, 12.217), 1e-3));
        CHECK(approx_eq(frame.cell().angles(), Vector3D(69.063, 70.888, 75.964), 1e-3));
        auto positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(5.0, 5.0, 5.0), 1e-2));

        frame = file.read();
        CHECK(frame.size() == 1);
        CHECK(frame.cell().shape() == UnitCell::TRICLINIC);
        CHECK(approx_eq(frame.cell().lengths(), Vector3D(10.0, 20.616, 12.217), 1e-3));
        CHECK(approx_eq(frame.cell().angles(), Vector3D(69.063, 70.888, 75.964), 1e-3));
        positions = frame.positions();
        CHECK(approx_eq(positions[0], Vector3D(44.0, 28.5, 16.0), 1e-3));
    }
}
