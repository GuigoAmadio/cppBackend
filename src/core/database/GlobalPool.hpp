#pragma once

#include <memory>
#include "ConnectionPool.hpp"

// Declaração externa do pool global definido em main_new.cpp
extern std::shared_ptr<Core::Database::ConnectionPool> globalPool;


