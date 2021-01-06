#include "flint/flint.h"
#include "flint/fmpz.h"
#include "flint/fmpz_poly.h"

#define slong mp_limb_signed_t



fmpz* get_coeff (fmpz_poly_t poly, slong i)
{
    static fmpz* zero;
    if (i >= 0 && i < poly->length)
        return poly->coeffs + i;
    else
    {
        if (zero == NULL)
        {
            zero = malloc(sizeof(fmpz));
            fmpz_zero(zero);
        }
        return zero;
    }
}


void get_slice(fmpz_poly_t slice, fmpz_poly_t poly, slong start, slong length)
{
    slong len = FLINT_MIN(length, poly->length - start);
    if (len < 0)
    {
        slice->coeffs = NULL;
        slice->alloc = 0;
        slice->length = 0;
    }
    else
    {
        slice->coeffs = poly->coeffs + start;
        slice->alloc = len;
        slice->length = len;
    }
}


void iadd_coeff(fmpz_poly_t poly, const fmpz_t summand, slong i)
{
    fmpz *coeff;
    slong len = i + 1;
    if (fmpz_is_zero(summand))
        return;

    if (poly->length < len)
        fmpz_poly_set_coeff_fmpz(poly, i, summand);
    else
    {
        coeff = poly->coeffs + i;
        fmpz_add(coeff, coeff, summand);
    }
}


void isub_coeff(fmpz_poly_t poly, const fmpz_t summand, slong i)
{
    fmpz *coeff;
    slong len = i + 1;
    if (fmpz_is_zero(summand))
        return;

    if (poly->length < len)
    {
        fmpz_poly_set_coeff_fmpz(poly, i, summand);
        coeff = poly->coeffs + i;
        fmpz_neg(coeff, coeff);
    }
    else
    {
        coeff = poly->coeffs + i;
        fmpz_sub(coeff, coeff, summand);
    }
}


void iadd_shifted(fmpz_poly_t poly, const fmpz_poly_t summand, slong shift)
{
    slong len = shift + summand->length;
    fmpz *cpoly, *last;
    fmpz* csummand = summand->coeffs;

    if (poly->length < len)
    {
        fmpz_poly_fit_length(poly, len);
        cpoly = poly->coeffs + shift;
        last = poly->coeffs + poly->length;
        for ( ; cpoly < last; cpoly++, csummand++)
            fmpz_add(cpoly, cpoly, csummand);
        last = poly->coeffs + shift + summand->length;
        for ( ; cpoly < last; cpoly++, csummand++)
            fmpz_set(cpoly, csummand);
        poly->length = len;
    }
    else
    {
        cpoly = poly->coeffs + shift;
        last = cpoly + summand->length;
        for ( ; cpoly < last; cpoly++, csummand++)
            fmpz_add(cpoly, cpoly, csummand);
    }
}


void reduce_coeff(fmpz_poly_t poly, slong i, const fmpz_t modulus)
{
    if (i < poly->length)
    {
        fmpz_t quo, rem;
        fmpz_init(quo); fmpz_init(rem);
        fmpz_tdiv_qr(quo, rem, poly->coeffs + i, modulus);
        if (fmpz_cmp_si(rem, 0) < 0)
        {
            fmpz_add(rem, rem, modulus);
            fmpz_sub_ui(quo, quo, 1);
        }
        iadd_coeff(poly, quo, i + 1);
        fmpz_poly_set_coeff_fmpz(poly, i, rem);
    }
}
