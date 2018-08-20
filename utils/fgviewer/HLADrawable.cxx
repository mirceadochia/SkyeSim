// Copyright (C) 2009 - 2012  Mathias Froehlich - Mathias.Froehlich@web.de
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "HLADrawable.hxx"

#include <simgear/hla/HLAArrayDataElement.hxx>
#include "HLADrawableClass.hxx"

namespace fgviewer {

HLADrawable::HLADrawable(HLADrawableClass* objectClass) :
    HLAObjectInstance(objectClass)
{
}

HLADrawable::~HLADrawable()
{
}

void
HLADrawable::createAttributeDataElements()
{
    /// FIXME at some point we should not need that anymore
    HLAObjectInstance::createAttributeDataElements();

    assert(dynamic_cast<HLADrawableClass*>(getObjectClass().get()));
    HLADrawableClass& objectClass = static_cast<HLADrawableClass&>(*getObjectClass());

    setAttributeDataElement(objectClass.getNameIndex(), _name.getDataElement());
    setAttributeDataElement(objectClass.getRendererIndex(), _renderer.getDataElement());
    setAttributeDataElement(objectClass.getDisplayIndex(), _display.getDataElement());
}

} // namespace fgviewer
