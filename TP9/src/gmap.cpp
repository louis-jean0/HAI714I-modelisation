//#include "gmap.hpp"
#ifdef GMAP_CORE
/*------------------------------------------------------------------------*/

/* 
    Create a new dart and return its id. 
    Set its alpha_i to itself (fixed points) 
    Use maxid to determine the new id. Dont forget to increment it.
*/
GMap::id_t GMap::add_dart()
{
    id_t dart = maxid;
    maxid++;
    alphas[dart] = alpha_container_t(dart,dart,dart);
    return dart;
}

/*------------------------------------------------------------------------*/


// Return the application of the alpha_deg on dart
GMap::id_t GMap::alpha(degree_t degree, id_t dart) const
{
    assert(degree < 3);
    assert(dart < maxid);
    return alphas.at(dart)[degree];
}

// Return the application of a composition of alphas on dart
GMap::id_t GMap::alpha(degreelist_t degrees, id_t dart) const
{
    std::reverse(degrees.begin(), degrees.end());
    for(degree_t degree : degrees){
        dart = alpha(degree,dart);
    }
    return dart;
}

//  Test if dart is free for alpha_degree (if it is a fixed point) 
bool GMap::is_free(degree_t degree, id_t dart) const
{
    assert(degree < 3);
    assert(dart < maxid);
    return alpha(degree,dart) == dart;
}

/*------------------------------------------------------------------------*/


// Link the two darts with a relation alpha_degree if they are both free.
bool GMap::link_darts(degree_t degree, id_t dart1, id_t dart2)
{
    if (!is_free(degree,dart1)) return false;
    if (!is_free(degree,dart2)) return false;

    alphas.at(dart1)[degree] = dart2;
    alphas.at(dart2)[degree] = dart1;

    return true;
}


/*------------------------------------------------------------------------*/

/*
        Test the validity of the structure. 
        Check if alpha_0 and alpha_1 are involutions with no fixed points.
        Check if alpha_2 is an involution.
        Check if alpha_0 o alpha_2 is an involution
*/
bool GMap::is_valid() const
{
    for(id_t dart : darts()) {
        if(alpha({0,0},dart) != dart) return false;
        if(alpha(0,dart) == dart) return false;
        if(alpha({1,1},dart) != dart) return false;
        if(alpha(1,dart) == dart) return false;
        if(alpha({2,2},dart) != dart) return false;
        if(alpha({0,2,0,2},dart) != dart) return false;
    }
    return true;
}

/*------------------------------------------------------------------------*/


/* 
    Return the orbit of dart using a list of alpha relation.
    Example of use : gmap.orbit([0,1],0).
*/
GMap::idlist_t GMap::orbit(const degreelist_t& alphas, id_t dart) const
{
    idlist_t result;
    idset_t marked;
    idlist_t toprocess = {dart};

    while (!toprocess.empty()){
        id_t d = toprocess.front();
        toprocess.erase(toprocess.begin());
        if (marked.count(d) == 0){
            result.push_back(d);
            marked.insert(d);
            for (degree_t degree : alphas) {
                toprocess.push_back(alpha(degree, d));
            }
        }

    }
    return result;
}

/*
    Return the ordered orbit of dart using a list of alpha relations by applying
    repeatingly the alpha relations of the list to dart.
    Example of use. gmap.orderedorbit([0,1],0).
    Warning: No fixed point for the given alpha should be contained.
*/
GMap::idlist_t GMap::orderedorbit(const degreelist_t& list_of_alpha_value, id_t dart) const
{
    idlist_t result;
    id_t current_dart = dart;
    unsigned char current_alpha_index = 0;
    size_t n_alpha = list_of_alpha_value.size();
    do{
            result.push_back(current_dart);
            degree_t current_alpha = list_of_alpha_value[current_alpha_index];
            current_dart = alpha(current_alpha, current_dart);
            current_alpha_index = (current_alpha_index+1) % n_alpha;
    } while (current_dart != dart);
    return result;
}


/*
    Sew two elements of degree 'degree' that start at dart1 and dart2.
    Determine first the orbits of dart to sew and heck if they are compatible.
    Sew pairs of corresponding darts
    # and if they have different embedding  positions, merge them. 
*/
bool GMap::sew_dart(degree_t degree, id_t dart1, id_t dart2)
{
    if (degree == 1) link_darts(1, dart1, dart2);
    
    else {

        degreelist_t alpha_list = {0};
        if (degree == 0) alpha_list = {2};

        idlist_t orbit1 = orbit(alpha_list, dart1);
        idlist_t orbit2 = orbit(alpha_list, dart2);

        if (orbit1.size() != orbit2.size())
            return false;

        idlist_t::const_iterator it1 =   orbit1.begin();
        idlist_t::const_iterator it2 =   orbit2.begin();
        for (; it1 != orbit1.end() ; ++it1, ++it2)
            link_darts(degree, *it1, *it2);
        }
    return true;
}

// Compute the Euler-Poincare characteristic of the subdivision
int GMap::eulercharacteristic() const
{
    int sign = 1;
    int characteristic = 0;
    for (unsigned char degree = 0 ; degree < 3 ; ++degree){
        characteristic += sign * elements(degree).size();
        sign *= -1;
    }
    return characteristic;
}

/*
    Check if a dart of the orbit representing the vertex has already been 
    associated with a value in propertydict. If yes, return this dart, else
    return the dart passed as argument.
*/

template<class T>
GMap::id_t EmbeddedGMap<T>::get_embedding_dart(id_t dart) const {
    for(id_t d : orbit({1,2}, dart)) {
        if(properties.count(d) == 1) {
            return d;
        }
    }
    return dart;
}

#endif
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

#ifdef GMAP_CORE

GMap3D GMap3D::dual()
{
    GMap3D dual_gmap;
    for (idalphamap_t::const_iterator it = alphas.begin(); it != alphas.end(); ++it){
            dual_gmap.alphas[it->first] = it->second.flip();
    }
    dual_gmap.maxid = maxid;

    for (id_t face_dart : elements(2)){
        vec3_t pos = element_center(2, face_dart);
        dual_gmap.set_position(face_dart,pos);
    }

    return dual_gmap;
}

/*------------------------------------------------------------------------*/

#endif