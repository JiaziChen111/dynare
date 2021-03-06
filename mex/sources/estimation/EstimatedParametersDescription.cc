/*
 * Copyright (C) 2010 Dynare Team
 *
 * This file is part of Dynare.
 *
 * Dynare is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dynare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Dynare.  If not, see <http://www.gnu.org/licenses/>.
 */

///////////////////////////////////////////////////////////
//  EstimatedParametersDescription.cpp
//  Implementation of the Class EstimatedParametersDescription
//  Created on:      02-Feb-2010 13:06:47
///////////////////////////////////////////////////////////

#include "EstimatedParametersDescription.hh"

EstimatedParametersDescription::~EstimatedParametersDescription()
{
}

EstimatedParametersDescription::EstimatedParametersDescription(std::vector<EstimationSubsample> &INestSubsamples, std::vector<EstimatedParameter> &INestParams) :
  estSubsamples(INestSubsamples), estParams(INestParams)
{
}

size_t
EstimatedParametersDescription::getNumberOfPeriods() const
{
  size_t r = 0;
  for (size_t i = 0; i < estSubsamples.size(); i++)
    {
      size_t r2 = estSubsamples[i].endPeriod + 1;
      if (r2 > r)
        r = r2;
    }
  return r;
}
